#include "../include/memory.h"

// Constantes para gerenciamento de memória
#define PAGE_SIZE 4096
#define FRAME_SIZE 4096
#define BITMAP_INDEX(frame) (frame / 32)
#define BITMAP_OFFSET(frame) (frame % 32)

// Variáveis globais
static physical_memory_manager_t pmm;
static virtual_memory_manager_t vmm;

// Inicializa o gerenciador de memória física
void pmm_init(u32 mem_upper) {
    // Calcula o número total de frames disponíveis
    // mem_upper é em KB, convertemos para bytes e dividimos pelo tamanho do frame
    pmm.total_frames = (mem_upper * 1024) / FRAME_SIZE;
    pmm.used_frames = 0;
    
    // Aloca espaço para o bitmap (1 bit por frame)
    // Cada entrada do bitmap controla 32 frames (32 bits)
    u32 bitmap_size = pmm.total_frames / 32;
    if (pmm.total_frames % 32) bitmap_size++;
    
    // Coloca o bitmap logo após o kernel
    // Endereço arbitrário para este exemplo, deve ser ajustado
    pmm.bitmap = (u32*)0x100000;
    
    // Inicializa o bitmap (todos os frames livres)
    for (u32 i = 0; i < bitmap_size; i++) {
        pmm.bitmap[i] = 0;
    }
    
    // Marca os primeiros frames como usados (kernel, bitmap, etc.)
    // Assumindo que o kernel ocupa os primeiros 1MB
    u32 kernel_frames = 256; // 1MB / 4KB = 256 frames
    for (u32 i = 0; i < kernel_frames; i++) {
        u32 idx = BITMAP_INDEX(i);
        u32 off = BITMAP_OFFSET(i);
        pmm.bitmap[idx] |= (1 << off);
        pmm.used_frames++;
    }
}

// Aloca um frame de memória física
void* pmm_alloc_frame() {
    if (pmm.used_frames >= pmm.total_frames) {
        return 0; // Sem memória disponível
    }
    
    // Procura por um frame livre
    for (u32 i = 0; i < pmm.total_frames / 32; i++) {
        if (pmm.bitmap[i] != 0xFFFFFFFF) { // Se não estiver completamente cheio
            // Encontra o primeiro bit livre
            for (u32 j = 0; j < 32; j++) {
                u32 bit = 1 << j;
                if (!(pmm.bitmap[i] & bit)) {
                    // Marca o frame como usado
                    pmm.bitmap[i] |= bit;
                    pmm.used_frames++;
                    
                    // Calcula o endereço físico
                    u32 frame = i * 32 + j;
                    return (void*)(frame * FRAME_SIZE);
                }
            }
        }
    }
    
    return 0; // Não deveria chegar aqui
}

// Libera um frame de memória física
void pmm_free_frame(void* frame_addr) {
    u32 frame = (u32)frame_addr / FRAME_SIZE;
    u32 idx = BITMAP_INDEX(frame);
    u32 off = BITMAP_OFFSET(frame);
    
    // Verifica se o frame está realmente alocado
    if (pmm.bitmap[idx] & (1 << off)) {
        pmm.bitmap[idx] &= ~(1 << off);
        pmm.used_frames--;
    }
}

// Estruturas para paginação
#define PD_INDEX(addr) ((addr >> 22) & 0x3FF)
#define PT_INDEX(addr) ((addr >> 12) & 0x3FF)
#define PAGE_PRESENT 0x1
#define PAGE_WRITE 0x2
#define PAGE_USER 0x4

// Inicializa o gerenciador de memória virtual
void vmm_init() {
    // Aloca um diretório de páginas
    vmm.page_directory = (u32*)pmm_alloc_frame();
    
    // Zera o diretório de páginas
    for (int i = 0; i < 1024; i++) {
        vmm.page_directory[i] = 0;
    }
}

// Mapeia uma página virtual para um endereço físico
void vmm_map_page(void* physical, void* virtual, u32 flags) {
    u32 pd_index = PD_INDEX((u32)virtual);
    u32 pt_index = PT_INDEX((u32)virtual);
    
    // Verifica se a tabela de páginas existe
    if (!(vmm.page_directory[pd_index] & PAGE_PRESENT)) {
        // Aloca uma nova tabela de páginas
        u32* page_table = (u32*)pmm_alloc_frame();
        
        // Zera a tabela de páginas
        for (int i = 0; i < 1024; i++) {
            page_table[i] = 0;
        }
        
        // Adiciona a tabela ao diretório
        vmm.page_directory[pd_index] = (u32)page_table | PAGE_PRESENT | PAGE_WRITE | flags;
    }
    
    // Obtém a tabela de páginas
    u32* page_table = (u32*)(vmm.page_directory[pd_index] & ~0xFFF);
    
    // Mapeia a página
    page_table[pt_index] = (u32)physical | PAGE_PRESENT | flags;
    
    // Atualiza o TLB
    asm volatile("invlpg (%0)" : : "r"(virtual));
}

// Desmapeia uma página virtual
void vmm_unmap_page(void* virtual) {
    u32 pd_index = PD_INDEX((u32)virtual);
    u32 pt_index = PT_INDEX((u32)virtual);
    
    // Verifica se a tabela de páginas existe
    if (vmm.page_directory[pd_index] & PAGE_PRESENT) {
        // Obtém a tabela de páginas
        u32* page_table = (u32*)(vmm.page_directory[pd_index] & ~0xFFF);
        
        // Desmapeia a página
        page_table[pt_index] = 0;
        
        // Atualiza o TLB
        asm volatile("invlpg (%0)" : : "r"(virtual));
    }
}

// Estrutura para o heap do kernel
#define HEAP_START 0xD0000000
#define HEAP_INITIAL_SIZE 0x100000 // 1MB

typedef struct {
    u32 magic;     // Assinatura para detectar corrupção
    u32 size;      // Tamanho do bloco
    u8 is_free;    // 1 se livre, 0 se alocado
} block_header_t;

static void* heap_start = (void*)HEAP_START;
static void* heap_end = (void*)(HEAP_START + HEAP_INITIAL_SIZE);
static block_header_t* first_block = NULL;

#define BLOCK_MAGIC 0xDEADBEEF

// Inicializa o heap do kernel
void heap_init() {
    // Mapeia as páginas para o heap inicial
    for (u32 addr = (u32)heap_start; addr < (u32)heap_end; addr += PAGE_SIZE) {
        void* physical = pmm_alloc_frame();
        vmm_map_page(physical, (void*)addr, PAGE_WRITE);
    }
    
    // Inicializa o primeiro bloco
    first_block = (block_header_t*)heap_start;
    first_block->magic = BLOCK_MAGIC;
    first_block->size = HEAP_INITIAL_SIZE - sizeof(block_header_t);
    first_block->is_free = 1;
}

// Aloca memória no heap do kernel
void* kmalloc(u32 size) {
    // Alinha o tamanho a 4 bytes
    if (size % 4 != 0) {
        size += 4 - (size % 4);
    }
    
    // Procura um bloco livre grande o suficiente
    block_header_t* current = first_block;
    while (current) {
        // Verifica a assinatura
        if (current->magic != BLOCK_MAGIC) {
            // Heap corrompido
            return NULL;
        }
        
        if (current->is_free && current->size >= size) {
            // Encontrou um bloco adequado
            
            // Verifica se vale a pena dividir o bloco
            if (current->size > size + sizeof(block_header_t) + 4) {
                // Divide o bloco
                block_header_t* new_block = (block_header_t*)((u32)current + sizeof(block_header_t) + size);
                new_block->magic = BLOCK_MAGIC;
                new_block->size = current->size - size - sizeof(block_header_t);
                new_block->is_free = 1;
                
                current->size = size;
            }
            
            current->is_free = 0;
            return (void*)((u32)current + sizeof(block_header_t));
        }
        
        // Avança para o próximo bloco
        if (current->size == 0) break; // Evita loop infinito
        current = (block_header_t*)((u32)current + sizeof(block_header_t) + current->size);
        
        // Verifica se chegou ao fim do heap
        if ((u32)current >= (u32)heap_end) {
            break;
        }
    }
    
    // Não encontrou bloco adequado
    return NULL;
}

// Libera memória no heap do kernel
void kfree(void* ptr) {
    if (!ptr) return;
    
    // Obtém o cabeçalho do bloco
    block_header_t* block = (block_header_t*)((u32)ptr - sizeof(block_header_t));
    
    // Verifica a assinatura
    if (block->magic != BLOCK_MAGIC) {
        // Ponteiro inválido
        return;
    }
    
    // Marca o bloco como livre
    block->is_free = 1;
    
    // Tenta mesclar com o próximo bloco se estiver livre
    block_header_t* next = (block_header_t*)((u32)block + sizeof(block_header_t) + block->size);
    if ((u32)next < (u32)heap_end && next->magic == BLOCK_MAGIC && next->is_free) {
        block->size += sizeof(block_header_t) + next->size;
    }
    
    // Tenta mesclar com o bloco anterior se estiver livre
    // (Isso exigiria uma lista duplamente encadeada ou uma varredura do início,
    // o que não é implementado neste exemplo simplificado)
}
