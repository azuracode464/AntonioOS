; context_switch.asm
; Implementação da troca de contexto entre processos

[BITS 32]
global context_switch

; void context_switch(cpu_state_t* old_state, cpu_state_t* new_state);
context_switch:
    ; Obtém os argumentos da pilha
    mov eax, [esp+4]   ; old_state
    mov edx, [esp+8]   ; new_state
    
    ; Salva o contexto atual no old_state
    mov [eax+0],  ebx  ; Salva ebx
    mov [eax+4],  ecx  ; Salva ecx
    mov [eax+8],  edx  ; Salva edx
    mov [eax+12], esi  ; Salva esi
    mov [eax+16], edi  ; Salva edi
    mov [eax+20], ebp  ; Salva ebp
    
    ; Salva esp
    mov ebx, esp
    add ebx, 8         ; Ajusta para o valor antes da chamada da função
    mov [eax+24], ebx  ; Salva esp
    
    ; Salva eip (endereço de retorno)
    mov ebx, [esp]
    mov [eax+28], ebx  ; Salva eip
    
    ; Salva eflags
    pushfd
    pop ebx
    mov [eax+32], ebx  ; Salva eflags
    
    ; Salva segmentos
    mov ebx, cs
    mov [eax+36], ebx  ; Salva cs
    mov ebx, ds
    mov [eax+40], ebx  ; Salva ds
    mov ebx, es
    mov [eax+44], ebx  ; Salva es
    mov ebx, fs
    mov [eax+48], ebx  ; Salva fs
    mov ebx, gs
    mov [eax+52], ebx  ; Salva gs
    mov ebx, ss
    mov [eax+56], ebx  ; Salva ss
    
    ; Restaura o contexto do new_state
    mov ebx, [edx+0]   ; Restaura ebx
    mov ecx, [edx+4]   ; Restaura ecx
    ; edx será restaurado depois
    mov esi, [edx+12]  ; Restaura esi
    mov edi, [edx+16]  ; Restaura edi
    mov ebp, [edx+20]  ; Restaura ebp
    
    ; Restaura segmentos
    mov eax, [edx+40]  ; Carrega ds
    mov ds, eax
    mov eax, [edx+44]  ; Carrega es
    mov es, eax
    mov eax, [edx+48]  ; Carrega fs
    mov fs, eax
    mov eax, [edx+52]  ; Carrega gs
    mov gs, eax
    
    ; Prepara para o iret
    mov eax, [edx+56]  ; ss
    push eax
    mov eax, [edx+24]  ; esp
    push eax
    mov eax, [edx+32]  ; eflags
    push eax
    mov eax, [edx+36]  ; cs
    push eax
    mov eax, [edx+28]  ; eip
    push eax
    
    ; Restaura edx
    mov edx, [edx+8]
    
    ; Retorna para o novo contexto
    iret
