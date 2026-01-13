# Makefile - Final Version
CC = gcc
ASM = nasm
LD = ld

# Compiler Flags (Force 32-bit, disable Linux features)
CFLAGS = -m32 -ffreestanding -O0 -Wall -Wextra -fno-stack-protector -fno-pie -mno-sse -mno-mmx -mno-80387
LDFLAGS = -m elf_i386 -T linker.ld

# Files
SOURCES = kernel.c
OBJECTS = boot.o kernel.o
TARGET_BIN = myos.bin
TARGET_ISO = myos.iso

all: $(TARGET_BIN)

# 1. Compile Assembly
boot.o: boot.s
	$(ASM) -felf32 boot.s -o boot.o

# 2. Compile C
kernel.o: $(SOURCES)
	$(CC) $(CFLAGS) -c $(SOURCES) -o kernel.o

# 3. Link Kernel
$(TARGET_BIN): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $(TARGET_BIN) $(OBJECTS)

# 4. Create ISO (For Ventoy)
iso: $(TARGET_BIN)
	mkdir -p isodir/boot/grub
	cp $(TARGET_BIN) isodir/boot/$(TARGET_BIN)
	echo 'menuentry "MyOS" { multiboot /boot/$(TARGET_BIN) }' > isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(TARGET_ISO) isodir

# 5. Run in QEMU
run: $(TARGET_BIN)
	qemu-system-i386 -kernel $(TARGET_BIN) -display gtk

# Cleanup
clean:
	rm -f *.o *.bin *.iso
	rm -rf isodir
