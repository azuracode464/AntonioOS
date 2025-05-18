# Arquitetura do Sistema Operacional

## Decisões de Arquitetura

### Plataforma de Hardware
- **Arquitetura**: x86 (32-bit)
  - Suporte a processadores Intel e AMD compatíveis
  - Inicialmente sem suporte a modo de 64 bits (x86_64)
  - Foco em compatibilidade com hardware comum

### Linguagens de Programação
- **Assembly (x86)**
  - Utilizado para: bootloader, rotinas de baixo nível, manipuladores de interrupção
  - Necessário para inicialização e operações que exigem controle direto do hardware
  - Sintaxe Intel será utilizada para maior clareza

- **C**
  - Linguagem principal para o kernel
  - Utilizada para: gerenciamento de memória, processos, sistema de arquivos
  - Escolhida por sua eficiência e proximidade ao hardware
  - Compilador: GCC (GNU Compiler Collection)

- **C++**
  - Utilizada para: componentes de mais alto nível do kernel
  - Implementação de estruturas de dados complexas
  - Orientação a objetos para drivers e subsistemas
  - Será utilizada com recursos limitados (sem exceções, RTTI limitado)

### Modelo de Kernel
- **Kernel Monolítico**
  - Todos os serviços do sistema operacional executam no espaço do kernel
  - Drivers integrados ao kernel
  - Maior eficiência de execução, embora com menor modularidade

### Modo de Operação
- **Modo Protegido (Protected Mode)**
  - Utilização dos 4 anéis de proteção do x86
  - Kernel no Ring 0, drivers no Ring 1, serviços no Ring 2, aplicações no Ring 3
  - Implementação de proteção de memória entre processos

### Gerenciamento de Memória
- **Paginação**
  - Páginas de 4KB
  - Suporte a memória virtual
  - Proteção de memória entre processos

### Sistema de Arquivos
- **Sistema de Arquivos Próprio**
  - Implementação de sistema simples inicialmente
  - Estrutura hierárquica de diretórios
  - Suporte a operações básicas (criar, ler, escrever, excluir)

### Interface com o Usuário
- **Inicialmente modo texto**
  - Shell de linha de comando
  - Interface baseada em texto 80x25 caracteres
  - Futura expansão para modo gráfico

## Ferramentas de Desenvolvimento

### Compilação
- **Cross-compiler GCC**
  - Configurado especificamente para o desenvolvimento do sistema operacional
  - Flags específicas para evitar dependências da libc

### Emulação/Teste
- **QEMU**
  - Emulação de hardware x86
  - Facilidade para depuração e teste

- **Bochs (alternativa)**
  - Emulador mais lento, porém com depuração mais detalhada

### Depuração
- **GDB**
  - Depuração remota via QEMU
  - Análise de código em execução

## Estrutura de Diretórios do Projeto

```
/
├── boot/           # Código do bootloader
├── kernel/         # Código-fonte do kernel
│   ├── arch/       # Código específico da arquitetura (x86)
│   ├── mm/         # Gerenciamento de memória
│   ├── proc/       # Gerenciamento de processos
│   ├── fs/         # Sistema de arquivos
│   ├── drivers/    # Drivers de dispositivos
│   └── include/    # Arquivos de cabeçalho
├── libc/           # Implementação mínima da biblioteca C
├── userland/       # Aplicativos de usuário
├── tools/          # Ferramentas de desenvolvimento
└── docs/           # Documentação
```

## Processo de Build

1. Compilação do bootloader (Assembly)
2. Compilação do kernel (C/C++/Assembly)
3. Criação da imagem do sistema
4. Teste em ambiente virtualizado

Esta arquitetura será refinada e expandida conforme o desenvolvimento avança, mas estabelece as bases fundamentais para o sistema operacional.
