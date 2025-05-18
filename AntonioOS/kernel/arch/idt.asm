; idt.asm
; Código para carregar a IDT

[BITS 32]
global idt_load     ; Torna a função visível externamente

idt_load:
    mov eax, [esp+4]  ; Obtém o ponteiro para a IDT
    lidt [eax]        ; Carrega a IDT
    ret
