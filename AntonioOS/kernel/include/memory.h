#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

// Definição de tipos
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Estrutura para entrada do mapa de memória
typedef struct {
    u32 size;
    u64 base_addr;
    u64 length;
    u32 type;
} __attribute__((packed)) memory_map_entry_t;

// Gerenciador de memória física
typedef struct {
    u32* bitmap;
    u32 total_frames;
    u32 used_frames;
} physical_memory_manager_t;

// Inicializa o gerenciador de memória física
void pmm_init(u32 mem_upper);

// Aloca um frame de memória física
void* pmm_alloc_frame();

// Libera um frame de memória física
void pmm_free_frame(void* frame);

// Gerenciador de memória virtual
typedef struct {
    u32* page_directory;
} virtual_memory_manager_t;

// Inicializa o gerenciador de memória virtual
void vmm_init();

// Mapeia uma página virtual para um endereço físico
void vmm_map_page(void* physical, void* virtual, u32 flags);

// Desmapeia uma página virtual
void vmm_unmap_page(void* virtual);

// Aloca memória no heap do kernel
void* kmalloc(u32 size);

// Libera memória no heap do kernel
void kfree(void* ptr);

#endif // MEMORY_H
