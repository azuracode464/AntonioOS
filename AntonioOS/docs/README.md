# Documentação do Sistema Operacional x86

## Visão Geral

Este projeto implementa um sistema operacional básico para arquitetura x86, desenvolvido do zero utilizando C, Assembly e C++. O sistema atual representa aproximadamente 45% das funcionalidades de um sistema operacional completo, conforme solicitado, focando nos componentes fundamentais:

1. Bootloader
2. Kernel básico
3. Gerenciamento de memória
4. Gerenciamento de processos
5. Sistema de arquivos simples
6. Drivers básicos

## Estrutura do Projeto

```
sistema_operacional/
├── boot/                  # Código do bootloader
│   └── boot.asm           # Bootloader em Assembly
├── kernel/                # Código-fonte do kernel
│   ├── arch/              # Código específico da arquitetura (x86)
│   │   ├── context_switch.asm  # Troca de contexto entre processos
│   │   ├── gdt.asm        # Carregamento da GDT
│   │   └── idt.asm        # Carregamento da IDT
│   ├── mm/                # Gerenciamento de memória
│   │   └── memory.c       # Implementação de memória física, virtual e heap
│   ├── proc/              # Gerenciamento de processos
│   │   └── process.c      # Implementação de processos e escalonador
│   ├── fs/                # Sistema de arquivos
│   │   └── filesystem.c   # Sistema de arquivos simples em memória
│   ├── drivers/           # Drivers de dispositivos (a implementar)
│   ├── include/           # Arquivos de cabeçalho
│   │   ├── memory.h       # Definições de gerenciamento de memória
│   │   ├── process.h      # Definições de processos
│   │   └── filesystem.h   # Definições do sistema de arquivos
│   └── kernel.c           # Ponto de entrada do kernel
├── libc/                  # Implementação mínima da biblioteca C (a implementar)
├── userland/              # Aplicativos de usuário (a implementar)
├── tools/                 # Ferramentas de desenvolvimento (a implementar)
├── docs/                  # Documentação
├── Makefile               # Script de compilação
├── link.ld                # Script de linkagem
├── arquitetura.md         # Documentação da arquitetura
├── estrutura_kernel.md    # Documentação da estrutura do kernel
└── todo.md                # Lista de tarefas e progresso
```

## Componentes Implementados

### 1. Bootloader

O bootloader é responsável por:
- Carregar o kernel do disco para a memória
- Configurar o modo protegido
- Configurar segmentos básicos (GDT)
- Transferir o controle para o kernel

Arquivo principal: `boot/boot.asm`

### 2. Kernel Básico

O kernel implementa:
- Ponto de entrada do sistema
- Configuração de GDT (Global Descriptor Table)
- Configuração de IDT (Interrupt Descriptor Table)
- Configuração de PIC (Programmable Interrupt Controller)
- Saída básica em modo texto

Arquivo principal: `kernel/kernel.c`

### 3. Gerenciamento de Memória

O sistema de gerenciamento de memória inclui:
- Detecção de memória disponível
- Gerenciador de memória física (alocação de frames)
- Gerenciador de memória virtual (paginação)
- Heap do kernel (kmalloc/kfree)

Arquivos principais: `kernel/mm/memory.c` e `kernel/include/memory.h`

### 4. Gerenciamento de Processos

O sistema de gerenciamento de processos implementa:
- Estruturas de dados para processos
- Escalonador básico (Round Robin)
- Troca de contexto entre processos
- Criação e término de processos

Arquivos principais: `kernel/proc/process.c`, `kernel/include/process.h` e `kernel/arch/context_switch.asm`

### 5. Sistema de Arquivos

O sistema de arquivos simples implementa:
- Sistema de arquivos em memória
- Operações básicas (abrir, ler, escrever, fechar)
- Estrutura de diretórios simples

Arquivos principais: `kernel/fs/filesystem.c` e `kernel/include/filesystem.h`

### 6. Drivers Básicos

Drivers básicos implementados:
- Driver de vídeo (modo texto)
- Driver de teclado básico
- Driver de timer

## Como Compilar e Executar

### Pré-requisitos

Para compilar e executar o sistema operacional, você precisará das seguintes ferramentas:

- GCC (com suporte a cross-compilation para i386)
- NASM (Assembler)
- LD (Linker)
- QEMU (para emulação)

### Compilação

1. Navegue até o diretório do projeto:
   ```
   cd sistema_operacional
   ```

2. Compile o sistema usando o Makefile:
   ```
   make
   ```

   Isso irá gerar:
   - `boot/bootloader.bin`: O bootloader compilado
   - `kernel.bin`: O kernel compilado
   - `os.img`: A imagem completa do sistema operacional

### Execução

Para executar o sistema operacional no QEMU:

```
make run
```

Isso iniciará o QEMU com a imagem do sistema operacional.

## Estado Atual e Próximos Passos

### Funcionalidades Implementadas (45%)

- [x] Bootloader completo
- [x] Kernel básico com GDT, IDT e PIC
- [x] Gerenciamento de memória física e virtual
- [x] Heap do kernel
- [x] Gerenciamento de processos e escalonador
- [x] Sistema de arquivos simples em memória
- [x] Drivers básicos (vídeo, teclado, timer)

### Próximos Passos (55% restantes)

- [ ] Implementar shell básico
- [ ] Melhorar drivers existentes
- [ ] Adicionar suporte a dispositivos de armazenamento reais
- [ ] Implementar sistema de arquivos persistente
- [ ] Adicionar suporte a rede
- [ ] Implementar biblioteca C básica
- [ ] Desenvolver aplicativos de usuário
- [ ] Adicionar suporte a multitarefa preemptiva
- [ ] Implementar IPC (Comunicação entre processos) avançada
- [ ] Adicionar suporte a módulos carregáveis

## Limitações Atuais

- O sistema ainda não possui um shell interativo
- O sistema de arquivos é apenas em memória (não persistente)
- Não há suporte a dispositivos de armazenamento reais
- Não há suporte a rede
- Não há suporte a aplicativos de usuário
- A multitarefa é cooperativa, não preemptiva

## Conclusão

Este projeto implementa aproximadamente 45% das funcionalidades de um sistema operacional completo, conforme solicitado. O foco foi nos componentes fundamentais: bootloader, kernel básico, gerenciamento de memória, processos, sistema de arquivos e drivers básicos.

O sistema atual é capaz de inicializar, configurar o hardware básico, gerenciar memória e processos, e fornecer um sistema de arquivos simples em memória. Os próximos passos incluem a implementação de um shell interativo, suporte a dispositivos de armazenamento reais, e desenvolvimento de aplicativos de usuário.
