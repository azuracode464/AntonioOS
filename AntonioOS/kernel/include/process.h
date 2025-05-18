#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

// Definição de tipos
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Estados possíveis de um processo
typedef enum {
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_TERMINATED
} process_state_t;

// Estrutura para armazenar o estado da CPU
typedef struct {
    u32 eax, ebx, ecx, edx;
    u32 esi, edi, ebp, esp;
    u32 eip, eflags;
    u32 cs, ds, es, fs, gs, ss;
} cpu_state_t;

// Estrutura de processo
typedef struct process {
    u32 pid;                      // ID do processo
    char name[32];                // Nome do processo
    process_state_t state;        // Estado atual
    u32* page_directory;          // Diretório de páginas
    void* kernel_stack;           // Pilha do kernel
    cpu_state_t* cpu_state;       // Estado da CPU salvo
    struct process* next;         // Próximo processo na lista
} process_t;

// Inicializa o sistema de processos
void process_init();

// Cria um novo processo
process_t* process_create(const char* name, void* entry_point);

// Termina um processo
void process_terminate(process_t* process);

// Bloqueia um processo
void process_block(process_t* process);

// Desbloqueia um processo
void process_unblock(process_t* process);

// Obtém o processo atual
process_t* process_get_current();

// Inicializa o escalonador
void scheduler_init();

// Adiciona um processo ao escalonador
void scheduler_add_process(process_t* process);

// Remove um processo do escalonador
void scheduler_remove_process(u32 pid);

// Realiza a troca de contexto
void scheduler_schedule();

// Função externa para troca de contexto (implementada em assembly)
extern void context_switch(cpu_state_t* old_state, cpu_state_t* new_state);

#endif // PROCESS_H
