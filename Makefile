# Makefile
# Compiler and Linker
CC = gcc
ASM = nasm
LD = ld

# Flags
# -m32: 32-bit mode
# -ffreestanding: No standard library
# -O0: No optimizations (easier debugging)
# -fno-*: Disable modern security features that require an OS
# -mno-*: Disable fancy CPU instructions (SSE/MMX)
CFLAGS = -m32 -ffreestanding -O0 -Wall -Wextra -fno-stack-protector -fno-pie -mno-sse -mno-mmx -mno-80387
LDFLAGS = -m elf_i386 -T linker.ld

# Files
SOURCES = kernel.c
OBJECTS = boot.o kernel.o
TARGET = myos.bin

# Default Rule
all: $(TARGET)

# Compile Assembly
boot.o: boot.s
	$(ASM) -felf32 boot.s -o boot.o

# Compile C
kernel.o: $(SOURCES)
	$(CC) $(CFLAGS) -c $(SOURCES) -o kernel.o

# Link
$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

# Run in QEMU
run: $(TARGET)
	qemu-system-i386 -kernel $(TARGET) -display gtk

# Clean up
clean:
	rm -f *.o $(TARGET)
