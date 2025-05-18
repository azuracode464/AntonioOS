# Estrutura do Kernel

## Visão Geral
Este documento descreve a estrutura inicial do kernel do nosso sistema operacional x86, detalhando os componentes principais e suas interações.

## Componentes Principais

### 1. Núcleo do Kernel (Core)

#### Ponto de Entrada
```c
// kernel_main.c
void kernel_main(multiboot_info_t* mbi) {
    // Inicialização do kernel
    init_gdt();        // Inicializa Global Descriptor Table
    init_idt();        // Inicializa Interrupt Descriptor Table
    init_pic();        // Configura Programmable Interrupt Controller
    init_memory(mbi);  // Inicializa gerenciamento de memória
    init_timer();      // Configura timer do sistema
    init_keyboard();   // Inicializa driver de teclado
    init_vga();        // Inicializa driver de vídeo
    
    // Inicializa escalonador e cria processo inicial
    scheduler_init();
    
    // Inicializa sistema de arquivos
    fs_init();
    
    // Inicia o shell
    start_shell();
    
    // Loop principal do kernel
    while(1) {
        halt_cpu();  // Pausa CPU até próxima interrupção
    }
}
```

#### Tabelas de Descritores
- **GDT (Global Descriptor Table)**: Define segmentos de memória e níveis de privilégio
- **IDT (Interrupt Descriptor Table)**: Mapeia vetores de interrupção para handlers

### 2. Gerenciamento de Memória

#### Detecção de Memória
- Utiliza informações do Multiboot para mapear memória disponível
- Cria mapa de memória física

#### Gerenciador de Memória Física
```c
// physical_memory.h
typedef struct {
    uint32_t* bitmap;
    uint32_t total_frames;
    uint32_t used_frames;
} physical_memory_manager_t;

void pmm_init(multiboot_info_t* mbi);
void* pmm_alloc_frame();
void pmm_free_frame(void* frame);
```

#### Gerenciador de Memória Virtual
```c
// virtual_memory.h
typedef struct {
    uint32_t* page_directory;
} virtual_memory_manager_t;

void vmm_init();
void vmm_map_page(void* physical, void* virtual, uint32_t flags);
void vmm_unmap_page(void* virtual);
```

#### Alocador de Memória do Kernel (Heap)
```c
// kmalloc.h
void* kmalloc(size_t size);
void* kcalloc(size_t num, size_t size);
void* krealloc(void* ptr, size_t size);
void kfree(void* ptr);
```

### 3. Gerenciamento de Processos

#### Estrutura de Processo
```c
// process.h
typedef enum {
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_TERMINATED
} process_state_t;

typedef struct {
    uint32_t pid;                  // ID do processo
    char name[32];                 // Nome do processo
    process_state_t state;         // Estado atual
    uint32_t* page_directory;      // Diretório de páginas
    void* kernel_stack;            // Pilha do kernel
    cpu_state_t* cpu_state;        // Estado da CPU salvo
    struct process_t* next;        // Próximo processo na lista
} process_t;
```

#### Escalonador
```c
// scheduler.h
void scheduler_init();
void scheduler_add_process(process_t* process);
void scheduler_remove_process(uint32_t pid);
void scheduler_schedule();
process_t* scheduler_get_current_process();
```

#### Troca de Contexto
```assembly
; context_switch.asm
global context_switch
context_switch:
    ; Salva contexto do processo atual
    push ebp
    mov ebp, esp
    
    ; Salva registradores
    push ebx
    push esi
    push edi
    
    ; Troca diretório de páginas
    ; ...
    
    ; Restaura registradores do novo processo
    ; ...
    
    ; Retorna para o novo processo
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
```

### 4. Sistema de Interrupções

#### Manipuladores de Interrupção
```c
// interrupt.h
typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} interrupt_frame_t;

void interrupt_handler(interrupt_frame_t* frame);
```

#### Controlador de Interrupções Programável (PIC)
```c
// pic.h
void pic_init();
void pic_send_eoi(uint8_t irq);
void pic_set_mask(uint8_t irq);
void pic_clear_mask(uint8_t irq);
```

### 5. Drivers Básicos

#### Driver de Teclado
```c
// keyboard.h
void keyboard_init();
char keyboard_get_char();
void keyboard_register_callback(void (*callback)(char));
```

#### Driver de Vídeo (Modo Texto)
```c
// vga.h
typedef enum {
    VGA_COLOR_BLACK,
    VGA_COLOR_BLUE,
    // ...
    VGA_COLOR_WHITE
} vga_color_t;

void vga_init();
void vga_clear_screen();
void vga_putchar(char c);
void vga_write(const char* str);
void vga_set_color(vga_color_t fg, vga_color_t bg);
void vga_set_cursor(int x, int y);
```

#### Driver de Timer
```c
// timer.h
void timer_init(uint32_t frequency);
uint32_t timer_get_ticks();
void timer_sleep(uint32_t ms);
void timer_register_callback(void (*callback)(void));
```

### 6. Sistema de Arquivos

#### Interface do Sistema de Arquivos
```c
// filesystem.h
typedef enum {
    FS_FILE,
    FS_DIRECTORY,
    FS_CHARDEVICE,
    FS_BLOCKDEVICE,
    FS_PIPE,
    FS_SYMLINK
} fs_node_type_t;

typedef struct fs_node {
    char name[128];
    fs_node_type_t type;
    uint32_t permissions;
    uint32_t size;
    
    // Funções de operação
    uint32_t (*read)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
    uint32_t (*write)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
    void (*open)(struct fs_node*);
    void (*close)(struct fs_node*);
    struct fs_node* (*readdir)(struct fs_node*, uint32_t);
    struct fs_node* (*finddir)(struct fs_node*, char* name);
} fs_node_t;

void fs_init();
uint32_t fs_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t fs_write(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
void fs_open(fs_node_t* node);
void fs_close(fs_node_t* node);
```

#### Sistema de Arquivos Simples
```c
// simplefs.h
void simplefs_init();
fs_node_t* simplefs_mount(uint32_t device_id);
```

### 7. Shell Básico

#### Interpretador de Comandos
```c
// shell.h
void shell_init();
void shell_run();
void shell_execute_command(const char* command);
```

## Fluxo de Inicialização

1. **Bootloader**:
   - Carrega kernel na memória
   - Configura modo protegido
   - Passa informações do Multiboot para o kernel
   - Transfere controle para kernel_main

2. **Kernel**:
   - Inicializa GDT e IDT
   - Configura PIC e interrupções
   - Inicializa gerenciamento de memória
   - Inicializa drivers básicos
   - Configura sistema de arquivos
   - Inicia o escalonador
   - Cria processo inicial (shell)
   - Entra em loop principal

## Diagrama de Interação dos Componentes

```
+----------------+     +----------------+     +----------------+
| Bootloader     |---->| Kernel Core    |---->| Gerenciamento  |
|                |     | (kernel_main)  |     | de Memória     |
+----------------+     +----------------+     +----------------+
                              |                      ^
                              v                      |
+----------------+     +----------------+     +----------------+
| Drivers        |<--->| Gerenciamento  |<--->| Sistema de     |
| de Hardware    |     | de Processos   |     | Arquivos       |
+----------------+     +----------------+     +----------------+
        ^                      |                      ^
        |                      v                      |
        |              +----------------+             |
        +------------->| Shell          |-------------+
                       |                |
                       +----------------+
```

## Próximos Passos

1. Implementar bootloader básico
2. Configurar ambiente de compilação cruzada
3. Implementar kernel minimalista com saída de texto
4. Adicionar suporte a interrupções
5. Implementar gerenciamento básico de memória
6. Adicionar suporte a processos simples
7. Implementar drivers básicos (teclado, vídeo)
8. Criar shell simples
