; boot.asm
; Bootloader para o sistema operacional x86
; Este código é carregado pelo BIOS e é responsável por carregar o kernel

[BITS 16]                       ; Modo 16 bits (real mode)
[ORG 0x7C00]                    ; Endereço de carregamento do bootloader

; Constantes
KERNEL_OFFSET equ 0x1000        ; Endereço onde o kernel será carregado

; Início do bootloader
start:
    ; Configuração inicial dos segmentos
    mov ax, 0x0000              ; Inicializa registradores de segmento
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00              ; Configura stack pointer

    ; Salva o drive de boot
    mov [BOOT_DRIVE], dl

    ; Mensagem de inicialização
    mov si, MSG_BOOT
    call print_string

    ; Carrega o kernel do disco
    call load_kernel

    ; Prepara para entrar no modo protegido
    call switch_to_pm

    ; Nunca deve chegar aqui
    jmp $

; Carrega o kernel do disco para a memória
load_kernel:
    mov si, MSG_LOAD_KERNEL
    call print_string

    mov bx, KERNEL_OFFSET       ; Endereço de destino
    mov dh, 15                  ; Número de setores a ler (ajuste conforme necessário)
    mov dl, [BOOT_DRIVE]        ; Drive de origem
    
    mov ah, 0x02                ; Função BIOS: ler setores
    mov al, dh                  ; Número de setores
    mov ch, 0                   ; Cilindro 0
    mov dh, 0                   ; Cabeça 0
    mov cl, 2                   ; Setor 2 (o setor 1 é o bootloader)
    int 0x13                    ; Chama BIOS
    
    jc disk_error               ; Verifica erro
    
    cmp al, dh                  ; Verifica se leu todos os setores
    jne disk_error
    
    ret

; Rotina de erro de disco
disk_error:
    mov si, MSG_DISK_ERROR
    call print_string
    jmp $                       ; Loop infinito

; Imprime uma string terminada em 0
print_string:
    pusha
    mov ah, 0x0E                ; Função BIOS: teletype
.loop:
    lodsb                       ; Carrega próximo caractere
    test al, al                 ; Verifica se é 0 (fim da string)
    jz .done
    int 0x10                    ; Chama BIOS para imprimir
    jmp .loop
.done:
    popa
    ret

; Prepara para entrar no modo protegido
switch_to_pm:
    cli                         ; Desativa interrupções
    
    ; Carrega GDT
    lgdt [gdt_descriptor]
    
    ; Habilita modo protegido
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Far jump para limpar pipeline
    jmp CODE_SEG:init_pm

[BITS 32]
; Inicialização no modo protegido
init_pm:
    ; Atualiza registradores de segmento
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Configura stack
    mov ebp, 0x90000
    mov esp, ebp
    
    ; Pula para o kernel
    call KERNEL_OFFSET
    
    ; Nunca deve chegar aqui
    jmp $

; Global Descriptor Table
gdt_start:
    ; GDT null descriptor
    dd 0x0
    dd 0x0
    
gdt_code:                       ; Segmento de código
    dw 0xFFFF                   ; Limite (0-15)
    dw 0x0                      ; Base (0-15)
    db 0x0                      ; Base (16-23)
    db 10011010b                ; Flags (P, DPL, S, Type)
    db 11001111b                ; Flags (G, D/B, L, AVL) + Limite (16-19)
    db 0x0                      ; Base (24-31)
    
gdt_data:                       ; Segmento de dados
    dw 0xFFFF                   ; Limite (0-15)
    dw 0x0                      ; Base (0-15)
    db 0x0                      ; Base (16-23)
    db 10010010b                ; Flags (P, DPL, S, Type)
    db 11001111b                ; Flags (G, D/B, L, AVL) + Limite (16-19)
    db 0x0                      ; Base (24-31)
    
gdt_end:

; Descritor da GDT
gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Tamanho da GDT
    dd gdt_start                ; Endereço da GDT

; Constantes para seletores de segmento
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; Variáveis
BOOT_DRIVE db 0

; Mensagens
MSG_BOOT db 'Bootloader iniciado...', 0
MSG_LOAD_KERNEL db 'Carregando kernel...', 0
MSG_DISK_ERROR db 'Erro ao ler disco!', 0

; Padding e assinatura de boot
times 510-($-$$) db 0
dw 0xAA55                       ; Assinatura de boot
