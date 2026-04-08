# =============================================================
# Genesi OS - Build System
# Requires: x86_64-elf-gcc, nasm, grub-mkrescue, qemu
# Run inside WSL2: make iso && make run
# =============================================================

CC     := x86_64-elf-gcc
LD     := x86_64-elf-ld
AS     := nasm
GRUB   := grub-mkrescue
QEMU   := qemu-system-x86_64

# -------------------------------------------------------------
# Compiler / Linker / Assembler flags
# -------------------------------------------------------------
CFLAGS := \
	-std=c11 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-builtin \
	-fno-pic \
	-mno-red-zone \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-Wall \
	-Wextra \
	-O2 \
	-g \
	-Ikernel/include

ASFLAGS := -f elf64 -g -F dwarf

LDFLAGS := -T linker.ld -nostdlib -z max-page-size=0x1000

# -------------------------------------------------------------
# Source discovery
# -------------------------------------------------------------
C_SRCS   := $(shell find kernel -name '*.c'   2>/dev/null)
ASM_SRCS := $(shell find kernel -name '*.asm' 2>/dev/null)

# -------------------------------------------------------------
# Object files
# -------------------------------------------------------------
BUILD_DIR := build

BOOT_OBJ := $(BUILD_DIR)/boot/boot.o
C_OBJS   := $(patsubst kernel/%.c,   $(BUILD_DIR)/kernel/%.o,     $(C_SRCS))
ASM_OBJS := $(patsubst kernel/%.asm, $(BUILD_DIR)/kernel/%.asm.o, $(ASM_SRCS))

ALL_OBJS := $(BOOT_OBJ) $(C_OBJS) $(ASM_OBJS)

# -------------------------------------------------------------
# Output targets
# -------------------------------------------------------------
KERNEL_ELF := $(BUILD_DIR)/genesi.elf
ISO_FILE   := genesi.iso

.PHONY: all iso run debug clean info

all: $(KERNEL_ELF)

# -------------------------------------------------------------
# Compilation rules
# -------------------------------------------------------------
$(BUILD_DIR)/boot/boot.o: boot/boot.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/kernel/%.o: kernel/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.asm.o: kernel/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# -------------------------------------------------------------
# Linking
# -------------------------------------------------------------
$(KERNEL_ELF): $(ALL_OBJS)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $(ALL_OBJS) -o $@
	@echo ""
	@echo "  [Genesi] Kernel linked: $@"
	@size $@

# -------------------------------------------------------------
# User Space Test Program
# -------------------------------------------------------------
USER_DIR := user/test
USER_ELF := $(BUILD_DIR)/user_test.elf

$(USER_ELF): $(USER_DIR)/main.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/user_main.o
	$(LD) -T $(USER_DIR)/linker.ld -nostdlib -z max-page-size=0x1000 $(BUILD_DIR)/user_main.o -o $@
	@echo "  [Genesi] User test linked: $@"

# -------------------------------------------------------------
# ISO image
# -------------------------------------------------------------
iso: $(KERNEL_ELF) $(USER_ELF)
	@mkdir -p iso/boot/grub iso/modules
	@cp $(KERNEL_ELF) iso/boot/genesi.elf
	@cp $(USER_ELF) iso/modules/test.elf
	@cp tools/grub.cfg iso/boot/grub/grub.cfg
	@$(GRUB) -o $(ISO_FILE) iso/ 2>/dev/null
	@echo "  [Genesi] ISO ready: $(ISO_FILE)"

# -------------------------------------------------------------
# QEMU
# -------------------------------------------------------------
QEMU_FLAGS := \
	-m 256M \
	-no-reboot \
	-no-shutdown \
	-vga std \
	-display sdl

run: iso
	$(QEMU) $(QEMU_FLAGS) -cdrom $(ISO_FILE)

# Run headless (for CI / WSL without display)
run-nographic: iso
	$(QEMU) -m 256M -no-reboot -no-shutdown \
	        -nographic -cdrom $(ISO_FILE)

test-headless: iso
	$(QEMU) -m 256M -no-reboot -no-shutdown -vga std -display none \
	        -serial file:serial.log -cdrom $(ISO_FILE)

# GDB debug session (start QEMU paused, attach gdb manually)
debug: iso
	$(QEMU) $(QEMU_FLAGS) -cdrom $(ISO_FILE) -s -S &
	gdb -ex "file $(KERNEL_ELF)" \
	    -ex "target remote localhost:1234" \
	    -ex "break kernel_main"

# -------------------------------------------------------------
# Clean
# -------------------------------------------------------------
clean:
	@rm -rf $(BUILD_DIR) iso/ $(ISO_FILE)
	@echo "  [Genesi] Clean done."

# -------------------------------------------------------------
# Info
# -------------------------------------------------------------
info:
	@echo "=== Genesi OS Build Info ==="
	@$(CC)   --version | head -1
	@$(LD)   --version | head -1
	@$(AS)   --version | head -1
	@$(QEMU) --version | head -1
