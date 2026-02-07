ARCH ?= x86_64
CROSS ?= x86_64-elf-

CC := $(CROSS)gcc
LD := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

CFLAGS := -ffreestanding -m64 -mno-red-zone -Wall -Wextra -O2 -Iinclude
ASFLAGS := -ffreestanding -m64
LDFLAGS := -T arch/$(ARCH)/linker.ld -nostdlib

BUILD_DIR := build
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_BIN := $(BUILD_DIR)/kernel.bin

OBJS := \
  $(BUILD_DIR)/boot.o \
  $(BUILD_DIR)/main.o \
  $(BUILD_DIR)/init.o \
  $(BUILD_DIR)/mm.o \
  $(BUILD_DIR)/process.o \
  $(BUILD_DIR)/scheduler.o \
  $(BUILD_DIR)/ipc.o \
  $(BUILD_DIR)/vfs.o \
  $(BUILD_DIR)/console.o \
  $(BUILD_DIR)/keyboard.o \
  $(BUILD_DIR)/vga.o

all: $(KERNEL_ELF) $(KERNEL_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/boot.o: arch/$(ARCH)/boot.s | $(BUILD_DIR)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/init.o: init.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/mm.o: mm.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/process.o: process.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/scheduler.o: scheduler.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ipc.o: ipc.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/vfs.o: vfs.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/console.o: console.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/keyboard.o: keyboard.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/vga.o: vga.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)

iso: all
	bash ./build_iso.sh

run: iso
	qemu-system-x86_64 -cdrom $(BUILD_DIR)/2026-os.iso

.PHONY: all clean iso run
