# Makefile para o Sistema Operacional x86

# Compiladores e ferramentas
CC = gcc
AS = nasm
LD = ld
QEMU = qemu-system-i386

# Flags de compilação
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T link.ld

# Diretórios
BOOT_DIR = boot
KERNEL_DIR = kernel
KERNEL_ARCH_DIR = $(KERNEL_DIR)/arch
KERNEL_MM_DIR = $(KERNEL_DIR)/mm
KERNEL_PROC_DIR = $(KERNEL_DIR)/proc
KERNEL_FS_DIR = $(KERNEL_DIR)/fs
KERNEL_DRIVERS_DIR = $(KERNEL_DIR)/drivers
KERNEL_INCLUDE_DIR = $(KERNEL_DIR)/include

# Arquivos de saída
BOOTLOADER = $(BOOT_DIR)/bootloader.bin
KERNEL = kernel.bin
OS_IMAGE = os.img

# Arquivos de origem
BOOT_SRC = $(BOOT_DIR)/boot.asm
KERNEL_C_SRC = $(KERNEL_DIR)/kernel.c \
               $(KERNEL_MM_DIR)/memory.c \
               $(KERNEL_PROC_DIR)/process.c \
               $(KERNEL_FS_DIR)/filesystem.c

KERNEL_ASM_SRC = $(KERNEL_ARCH_DIR)/gdt.asm \
                 $(KERNEL_ARCH_DIR)/idt.asm \
                 $(KERNEL_ARCH_DIR)/context_switch.asm

# Arquivos objeto
KERNEL_C_OBJ = $(KERNEL_C_SRC:.c=.o)
KERNEL_ASM_OBJ = $(KERNEL_ASM_SRC:.asm=.o)
KERNEL_OBJ = $(KERNEL_C_OBJ) $(KERNEL_ASM_OBJ)

# Regras
all: $(OS_IMAGE)

# Cria a imagem do sistema operacional
$(OS_IMAGE): $(BOOTLOADER) $(KERNEL)
	@echo "Criando imagem do sistema operacional..."
	dd if=/dev/zero of=$(OS_IMAGE) bs=512 count=2880
	dd if=$(BOOTLOADER) of=$(OS_IMAGE) conv=notrunc
	dd if=$(KERNEL) of=$(OS_IMAGE) seek=1 conv=notrunc

# Compila o bootloader
$(BOOTLOADER): $(BOOT_SRC)
	@echo "Compilando bootloader..."
	$(AS) -f bin $(BOOT_SRC) -o $(BOOTLOADER)

# Compila o kernel
$(KERNEL): $(KERNEL_OBJ) link.ld
	@echo "Linkando kernel..."
	$(LD) $(LDFLAGS) -o $(KERNEL) $(KERNEL_OBJ)

# Compila arquivos C
%.o: %.c
	@echo "Compilando $<..."
	$(CC) $(CFLAGS) -I$(KERNEL_INCLUDE_DIR) $< -o $@

# Compila arquivos Assembly
%.o: %.asm
	@echo "Compilando $<..."
	$(AS) $(ASFLAGS) $< -o $@

# Executa o sistema operacional no QEMU
run: $(OS_IMAGE)
	@echo "Executando sistema operacional no QEMU..."
	$(QEMU) -fda $(OS_IMAGE)

# Limpa arquivos gerados
clean:
	@echo "Limpando arquivos gerados..."
	rm -f $(BOOTLOADER) $(KERNEL) $(KERNEL_OBJ) $(OS_IMAGE)

.PHONY: all run clean
