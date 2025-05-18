#include <stdint.h>

// Definição de tipos
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Estrutura para informações do Multiboot
typedef struct {
    u32 flags;
    u32 mem_lower;
    u32 mem_upper;
    u32 boot_device;
    u32 cmdline;
    u32 mods_count;
    u32 mods_addr;
    u32 syms[4];
    u32 mmap_length;
    u32 mmap_addr;
    u32 drives_length;
    u32 drives_addr;
    u32 config_table;
    u32 boot_loader_name;
    u32 apm_table;
    u32 vbe_control_info;
    u32 vbe_mode_info;
    u16 vbe_mode;
    u16 vbe_interface_seg;
    u16 vbe_interface_off;
    u16 vbe_interface_len;
} multiboot_info_t;

// Funções de E/S
static inline void outb(u16 port, u8 value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port) {
    u8 ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Funções de vídeo (modo texto)
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

u16* vga_buffer = (u16*)VGA_MEMORY;
u8 cursor_x = 0;
u8 cursor_y = 0;
u8 vga_color = 0x0F; // Branco sobre preto

void vga_clear() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (u16)(' ' | (vga_color << 8));
    }
    cursor_x = 0;
    cursor_y = 0;
}

void vga_set_color(u8 fg, u8 bg) {
    vga_color = (bg << 4) | (fg & 0x0F);
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            // Scroll
            for (int y = 0; y < VGA_HEIGHT - 1; y++) {
                for (int x = 0; x < VGA_WIDTH; x++) {
                    vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
                }
            }
            // Limpa última linha
            for (int x = 0; x < VGA_WIDTH; x++) {
                vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (u16)(' ' | (vga_color << 8));
            }
            cursor_y = VGA_HEIGHT - 1;
        }
    } else if (c == '\r') {
        cursor_x = 0;
    } else {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (u16)(c | (vga_color << 8));
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
            if (cursor_y >= VGA_HEIGHT) {
                // Scroll (mesmo código de acima)
                for (int y = 0; y < VGA_HEIGHT - 1; y++) {
                    for (int x = 0; x < VGA_WIDTH; x++) {
                        vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
                    }
                }
                // Limpa última linha
                for (int x = 0; x < VGA_WIDTH; x++) {
                    vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (u16)(' ' | (vga_color << 8));
                }
                cursor_y = VGA_HEIGHT - 1;
            }
        }
    }
}

void vga_write(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

// GDT (Global Descriptor Table)
struct gdt_entry {
    u16 limit_low;
    u16 base_low;
    u8 base_middle;
    u8 access;
    u8 granularity;
    u8 base_high;
} __attribute__((packed));

struct gdt_ptr {
    u16 limit;
    u32 base;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gp;

extern void gdt_flush(u32);

void gdt_set_gate(int num, u32 base, u32 limit, u8 access, u8 gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    
    gdt[num].access = access;
}

void gdt_init() {
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (u32)&gdt;
    
    // Null descriptor
    gdt_set_gate(0, 0, 0, 0, 0);
    
    // Code segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    
    // Data segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    
    // Flush GDT
    gdt_flush((u32)&gp);
}

// IDT (Interrupt Descriptor Table)
struct idt_entry {
    u16 base_lo;
    u16 sel;
    u8 always0;
    u8 flags;
    u16 base_hi;
} __attribute__((packed));

struct idt_ptr {
    u16 limit;
    u32 base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void idt_load(u32);

void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_init() {
    idtp.limit = sizeof(struct idt_entry) * 256 - 1;
    idtp.base = (u32)&idt;
    
    // Zera a IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    // Configura as interrupções básicas
    // (Aqui seriam configurados os handlers de interrupção)
    
    // Carrega a IDT
    idt_load((u32)&idtp);
}

// PIC (Programmable Interrupt Controller)
#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

void pic_init() {
    // ICW1: Inicialização
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);
    
    // ICW2: Remapeamento
    outb(PIC1_DATA, 0x20); // IRQ 0-7 -> INT 0x20-0x27
    outb(PIC2_DATA, 0x28); // IRQ 8-15 -> INT 0x28-0x2F
    
    // ICW3: Cascateamento
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    
    // ICW4: Modo 8086
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    
    // Mascara todas as interrupções exceto teclado (IRQ1)
    outb(PIC1_DATA, 0xFD);
    outb(PIC2_DATA, 0xFF);
}

// Função principal do kernel
void kernel_main(multiboot_info_t* mbi) {
    // Inicializa subsistemas
    vga_clear();
    gdt_init();
    idt_init();
    pic_init();
    
    // Mensagem de boas-vindas
    vga_write("Kernel inicializado com sucesso!\n");
    vga_write("Sistema Operacional x86 - Versao 0.1\n");
    vga_write("----------------------------------------\n");
    
    // Informações de memória do Multiboot
    vga_write("Informacoes de memoria:\n");
    vga_write("  Memoria baixa: ");
    // Aqui seria adicionado código para converter e exibir mbi->mem_lower
    vga_write(" KB\n");
    
    vga_write("  Memoria alta: ");
    // Aqui seria adicionado código para converter e exibir mbi->mem_upper
    vga_write(" KB\n\n");
    
    vga_write("Sistema em modo de espera...\n");
    
    // Loop infinito
    while (1) {
        // Halt CPU até próxima interrupção
        asm volatile("hlt");
    }
}
