#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>

// Definição de tipos
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Tipos de nós do sistema de arquivos
typedef enum {
    FS_FILE,
    FS_DIRECTORY,
    FS_CHARDEVICE,
    FS_BLOCKDEVICE,
    FS_PIPE,
    FS_SYMLINK
} fs_node_type_t;

// Estrutura para nós do sistema de arquivos
typedef struct fs_node {
    char name[128];                  // Nome do arquivo/diretório
    fs_node_type_t type;             // Tipo do nó
    u32 permissions;                 // Permissões (rwx)
    u32 uid;                         // ID do usuário
    u32 gid;                         // ID do grupo
    u32 size;                        // Tamanho em bytes
    u32 inode;                       // Número do inode
    u32 impl;                        // Implementação específica
    
    // Funções de operação
    u32 (*read)(struct fs_node*, u32, u32, u8*);
    u32 (*write)(struct fs_node*, u32, u32, u8*);
    void (*open)(struct fs_node*);
    void (*close)(struct fs_node*);
    struct fs_node* (*readdir)(struct fs_node*, u32);
    struct fs_node* (*finddir)(struct fs_node*, char*);
} fs_node_t;

// Inicializa o sistema de arquivos
void fs_init();

// Funções de operação de arquivos
u32 fs_read(fs_node_t* node, u32 offset, u32 size, u8* buffer);
u32 fs_write(fs_node_t* node, u32 offset, u32 size, u8* buffer);
void fs_open(fs_node_t* node);
void fs_close(fs_node_t* node);
fs_node_t* fs_readdir(fs_node_t* node, u32 index);
fs_node_t* fs_finddir(fs_node_t* node, char* name);

// Sistema de arquivos raiz
extern fs_node_t* fs_root;

#endif // FILESYSTEM_H
