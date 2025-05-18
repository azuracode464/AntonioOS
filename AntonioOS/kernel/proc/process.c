#include "../include/process.h"
#include "../include/memory.h"

// Lista de processos
static process_t* process_list = NULL;
static process_t* current_process = NULL;
static u32 next_pid = 1;

// Tamanho da pilha do kernel para cada processo
#define KERNEL_STACK_SIZE 4096

// Inicializa o sistema de processos
void process_init() {
    // Inicializa a lista de processos
    process_list = NULL;
    current_process = NULL;
}

// Cria um novo processo
process_t* process_create(const char* name, void* entry_point) {
    // Aloca memória para o processo
    process_t* process = (process_t*)kmalloc(sizeof(process_t));
    if (!process) return NULL;
    
    // Inicializa os campos básicos
    process->pid = next_pid++;
    for (int i = 0; i < 31 && name[i]; i++) {
        process->name[i] = name[i];
    }
    process->name[31] = '\0';
    process->state = PROCESS_STATE_READY;
    
    // Cria um diretório de páginas para o processo
    // (Simplificado: usa o diretório do kernel para todos os processos)
    process->page_directory = (u32*)0x1000; // Endereço arbitrário para exemplo
    
    // Aloca pilha do kernel para o processo
    process->kernel_stack = kmalloc(KERNEL_STACK_SIZE);
    if (!process->kernel_stack) {
        kfree(process);
        return NULL;
    }
    
    // Configura o estado inicial da CPU
    process->cpu_state = (cpu_state_t*)((u32)process->kernel_stack + KERNEL_STACK_SIZE - sizeof(cpu_state_t));
    
    // Zera o estado da CPU
    for (int i = 0; i < sizeof(cpu_state_t) / 4; i++) {
        ((u32*)process->cpu_state)[i] = 0;
    }
    
    // Configura registradores para o ponto de entrada
    process->cpu_state->eip = (u32)entry_point;
    process->cpu_state->cs = 0x08; // Seletor de segmento de código
    process->cpu_state->ds = 0x10; // Seletor de segmento de dados
    process->cpu_state->es = 0x10;
    process->cpu_state->fs = 0x10;
    process->cpu_state->gs = 0x10;
    process->cpu_state->ss = 0x10;
    process->cpu_state->eflags = 0x202; // IF=1, bit reservado=1
    
    // Adiciona o processo à lista
    process->next = process_list;
    process_list = process;
    
    return process;
}

// Termina um processo
void process_terminate(process_t* process) {
    if (!process) return;
    
    // Marca o processo como terminado
    process->state = PROCESS_STATE_TERMINATED;
    
    // Remove o processo da lista (simplificado)
    if (process_list == process) {
        process_list = process->next;
    } else {
        process_t* prev = process_list;
        while (prev && prev->next != process) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = process->next;
        }
    }
    
    // Libera recursos
    kfree(process->kernel_stack);
    kfree(process);
}

// Bloqueia um processo
void process_block(process_t* process) {
    if (!process) return;
    process->state = PROCESS_STATE_BLOCKED;
}

// Desbloqueia um processo
void process_unblock(process_t* process) {
    if (!process) return;
    if (process->state == PROCESS_STATE_BLOCKED) {
        process->state = PROCESS_STATE_READY;
    }
}

// Obtém o processo atual
process_t* process_get_current() {
    return current_process;
}

// Inicializa o escalonador
void scheduler_init() {
    // Inicializa o sistema de processos
    process_init();
    
    // Cria o processo inicial (idle)
    process_t* idle = process_create("idle", NULL); // NULL será substituído pelo ponto de entrada real
    
    // Define o processo atual
    current_process = idle;
    current_process->state = PROCESS_STATE_RUNNING;
}

// Adiciona um processo ao escalonador
void scheduler_add_process(process_t* process) {
    // O processo já foi adicionado à lista em process_create
    // Aqui poderíamos adicionar lógica adicional se necessário
}

// Remove um processo do escalonador
void scheduler_remove_process(u32 pid) {
    // Procura o processo com o PID especificado
    process_t* process = process_list;
    while (process) {
        if (process->pid == pid) {
            process_terminate(process);
            return;
        }
        process = process->next;
    }
}

// Realiza a troca de contexto
void scheduler_schedule() {
    // Se não há processos, retorna
    if (!process_list) return;
    
    // Salva o processo atual
    process_t* old_process = current_process;
    
    // Encontra o próximo processo a ser executado
    process_t* next = NULL;
    
    // Estratégia simples de Round Robin
    if (current_process) {
        next = current_process->next;
    }
    
    // Se chegou ao fim da lista ou não há processo atual, volta ao início
    if (!next) {
        next = process_list;
    }
    
    // Procura um processo que esteja pronto
    process_t* start = next;
    do {
        if (next->state == PROCESS_STATE_READY) {
            break;
        }
        next = next->next;
        if (!next) next = process_list;
    } while (next != start);
    
    // Se não encontrou nenhum processo pronto, mantém o atual
    if (next->state != PROCESS_STATE_READY) {
        return;
    }
    
    // Atualiza o estado dos processos
    if (current_process && current_process->state == PROCESS_STATE_RUNNING) {
        current_process->state = PROCESS_STATE_READY;
    }
    next->state = PROCESS_STATE_RUNNING;
    
    // Atualiza o processo atual
    current_process = next;
    
    // Realiza a troca de contexto
    if (old_process != current_process) {
        context_switch(old_process->cpu_state, current_process->cpu_state);
    }
}
