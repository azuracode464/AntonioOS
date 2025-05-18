#include "../include/filesystem.h"
#include "../include/memory.h"

// Nó raiz do sistema de arquivos
fs_node_t* fs_root = NULL;

// Sistema de arquivos simples em memória
typedef struct {
    char name[128];
    u8* data;
    u32 size;
    u32 capacity;
} simplefs_file_t;

#define MAX_FILES 64
static simplefs_file_t files[MAX_FILES];
static u32 file_count = 0;

// Inicializa o sistema de arquivos
void fs_init() {
    // Inicializa o array de arquivos
    for (u32 i = 0; i < MAX_FILES; i++) {
        files[i].name[0] = '\0';
        files[i].data = NULL;
        files[i].size = 0;
        files[i].capacity = 0;
    }
    
    // Cria o nó raiz
    fs_root = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (!fs_root) return;
    
    // Inicializa o nó raiz
    for (int i = 0; i < 127; i++) {
        fs_root->name[i] = '/';
        fs_root->name[i+1] = '\0';
    }
    fs_root->type = FS_DIRECTORY;
    fs_root->permissions = 0755; // rwxr-xr-x
    fs_root->uid = 0;
    fs_root->gid = 0;
    fs_root->size = 0;
    fs_root->inode = 0;
    fs_root->impl = 0;
    
    // Configura as funções de operação
    fs_root->read = NULL;
    fs_root->write = NULL;
    fs_root->open = NULL;
    fs_root->close = NULL;
    fs_root->readdir = simplefs_readdir;
    fs_root->finddir = simplefs_finddir;
}

// Lê um arquivo
u32 simplefs_read(fs_node_t* node, u32 offset, u32 size, u8* buffer) {
    // Verifica se o nó é válido
    if (!node || node->type != FS_FILE) return 0;
    
    // Obtém o índice do arquivo
    u32 index = node->impl;
    if (index >= MAX_FILES || !files[index].data) return 0;
    
    // Verifica os limites
    if (offset >= files[index].size) return 0;
    if (offset + size > files[index].size) {
        size = files[index].size - offset;
    }
    
    // Copia os dados
    for (u32 i = 0; i < size; i++) {
        buffer[i] = files[index].data[offset + i];
    }
    
    return size;
}

// Escreve em um arquivo
u32 simplefs_write(fs_node_t* node, u32 offset, u32 size, u8* buffer) {
    // Verifica se o nó é válido
    if (!node || node->type != FS_FILE) return 0;
    
    // Obtém o índice do arquivo
    u32 index = node->impl;
    if (index >= MAX_FILES) return 0;
    
    // Se o arquivo não existe, cria-o
    if (!files[index].data) {
        files[index].capacity = offset + size;
        files[index].data = (u8*)kmalloc(files[index].capacity);
        if (!files[index].data) return 0;
        files[index].size = 0;
    }
    
    // Se o offset + size excede a capacidade, realoca
    if (offset + size > files[index].capacity) {
        u32 new_capacity = offset + size;
        u8* new_data = (u8*)kmalloc(new_capacity);
        if (!new_data) return 0;
        
        // Copia os dados existentes
        for (u32 i = 0; i < files[index].size; i++) {
            new_data[i] = files[index].data[i];
        }
        
        // Libera o buffer antigo
        kfree(files[index].data);
        
        // Atualiza o arquivo
        files[index].data = new_data;
        files[index].capacity = new_capacity;
    }
    
    // Copia os dados
    for (u32 i = 0; i < size; i++) {
        files[index].data[offset + i] = buffer[i];
    }
    
    // Atualiza o tamanho se necessário
    if (offset + size > files[index].size) {
        files[index].size = offset + size;
        node->size = files[index].size;
    }
    
    return size;
}

// Abre um arquivo
void simplefs_open(fs_node_t* node) {
    // Nada a fazer nesta implementação simples
}

// Fecha um arquivo
void simplefs_close(fs_node_t* node) {
    // Nada a fazer nesta implementação simples
}

// Lê um diretório
fs_node_t* simplefs_readdir(fs_node_t* node, u32 index) {
    // Verifica se o nó é um diretório
    if (!node || node->type != FS_DIRECTORY) return NULL;
    
    // Verifica se o índice é válido
    if (index >= file_count) return NULL;
    
    // Cria um nó para o arquivo
    fs_node_t* file_node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (!file_node) return NULL;
    
    // Inicializa o nó
    for (int i = 0; i < 127 && files[index].name[i]; i++) {
        file_node->name[i] = files[index].name[i];
        file_node->name[i+1] = '\0';
    }
    file_node->type = FS_FILE;
    file_node->permissions = 0644; // rw-r--r--
    file_node->uid = 0;
    file_node->gid = 0;
    file_node->size = files[index].size;
    file_node->inode = index + 1;
    file_node->impl = index;
    
    // Configura as funções de operação
    file_node->read = simplefs_read;
    file_node->write = simplefs_write;
    file_node->open = simplefs_open;
    file_node->close = simplefs_close;
    file_node->readdir = NULL;
    file_node->finddir = NULL;
    
    return file_node;
}

// Encontra um arquivo em um diretório
fs_node_t* simplefs_finddir(fs_node_t* node, char* name) {
    // Verifica se o nó é um diretório
    if (!node || node->type != FS_DIRECTORY) return NULL;
    
    // Procura o arquivo pelo nome
    for (u32 i = 0; i < file_count; i++) {
        // Compara os nomes
        int j;
        for (j = 0; name[j] && files[i].name[j]; j++) {
            if (name[j] != files[i].name[j]) break;
        }
        
        // Se os nomes são iguais
        if (!name[j] && !files[i].name[j]) {
            // Cria um nó para o arquivo (mesmo código de readdir)
            fs_node_t* file_node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
            if (!file_node) return NULL;
            
            // Inicializa o nó
            for (int k = 0; k < 127 && files[i].name[k]; k++) {
                file_node->name[k] = files[i].name[k];
                file_node->name[k+1] = '\0';
            }
            file_node->type = FS_FILE;
            file_node->permissions = 0644; // rw-r--r--
            file_node->uid = 0;
            file_node->gid = 0;
            file_node->size = files[i].size;
            file_node->inode = i + 1;
            file_node->impl = i;
            
            // Configura as funções de operação
            file_node->read = simplefs_read;
            file_node->write = simplefs_write;
            file_node->open = simplefs_open;
            file_node->close = simplefs_close;
            file_node->readdir = NULL;
            file_node->finddir = NULL;
            
            return file_node;
        }
    }
    
    return NULL;
}

// Cria um novo arquivo
fs_node_t* simplefs_create(const char* name) {
    // Verifica se já existe um arquivo com esse nome
    for (u32 i = 0; i < file_count; i++) {
        // Compara os nomes
        int j;
        for (j = 0; name[j] && files[i].name[j]; j++) {
            if (name[j] != files[i].name[j]) break;
        }
        
        // Se os nomes são iguais
        if (!name[j] && !files[i].name[j]) {
            return NULL; // Arquivo já existe
        }
    }
    
    // Verifica se há espaço para um novo arquivo
    if (file_count >= MAX_FILES) return NULL;
    
    // Cria o novo arquivo
    u32 index = file_count++;
    
    // Copia o nome
    for (int i = 0; i < 127 && name[i]; i++) {
        files[index].name[i] = name[i];
        files[index].name[i+1] = '\0';
    }
    
    // Inicializa o arquivo
    files[index].data = NULL;
    files[index].size = 0;
    files[index].capacity = 0;
    
    // Cria um nó para o arquivo
    fs_node_t* file_node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (!file_node) {
        file_count--; // Reverte a criação
        return NULL;
    }
    
    // Inicializa o nó
    for (int i = 0; i < 127 && name[i]; i++) {
        file_node->name[i] = name[i];
        file_node->name[i+1] = '\0';
    }
    file_node->type = FS_FILE;
    file_node->permissions = 0644; // rw-r--r--
    file_node->uid = 0;
    file_node->gid = 0;
    file_node->size = 0;
    file_node->inode = index + 1;
    file_node->impl = index;
    
    // Configura as funções de operação
    file_node->read = simplefs_read;
    file_node->write = simplefs_write;
    file_node->open = simplefs_open;
    file_node->close = simplefs_close;
    file_node->readdir = NULL;
    file_node->finddir = NULL;
    
    return file_node;
}

// Funções de wrapper para o sistema de arquivos
u32 fs_read(fs_node_t* node, u32 offset, u32 size, u8* buffer) {
    if (!node || !node->read) return 0;
    return node->read(node, offset, size, buffer);
}

u32 fs_write(fs_node_t* node, u32 offset, u32 size, u8* buffer) {
    if (!node || !node->write) return 0;
    return node->write(node, offset, size, buffer);
}

void fs_open(fs_node_t* node) {
    if (!node || !node->open) return;
    node->open(node);
}

void fs_close(fs_node_t* node) {
    if (!node || !node->close) return;
    node->close(node);
}

fs_node_t* fs_readdir(fs_node_t* node, u32 index) {
    if (!node || !node->readdir) return NULL;
    return node->readdir(node, index);
}

fs_node_t* fs_finddir(fs_node_t* node, char* name) {
    if (!node || !node->finddir) return NULL;
    return node->finddir(node, name);
}
