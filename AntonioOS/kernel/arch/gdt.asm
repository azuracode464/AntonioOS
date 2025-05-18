; gdt.asm
; Código para carregar a GDT

[BITS 32]
global gdt_flush    ; Torna a função visível externamente

gdt_flush:
    mov eax, [esp+4]  ; Obtém o ponteiro para a GDT
    lgdt [eax]        ; Carrega a GDT
    
    ; Atualiza os registradores de segmento
    mov ax, 0x10      ; 0x10 é o offset para o segmento de dados na GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Far jump para atualizar CS (segmento de código)
    jmp 0x08:.flush   ; 0x08 é o offset para o segmento de código na GDT
.flush:
    ret
