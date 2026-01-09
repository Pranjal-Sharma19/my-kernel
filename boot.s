; boot.s
; This file sets up the environment for C

; 1. Define Multiboot Header Constants
MBALIGN  equ  1 << 0            ; Align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; Provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'Magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)   ; Checksum required

; 2. The Multiboot Header (Must be at the very start)
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; 3. The Stack (C needs a stack to work)
section .bss
align 16
stack_bottom:
    resb 16384 ; Reserve 16KB for the stack
stack_top:

; 4. The Entry Point
section .text
global _start:function (_start.end - _start)
    ; This symbol is defined in kernel.c
    extern kernel_main 

_start:
    ; Set up the stack pointer (esp)
    mov esp, stack_top

    ; Call the C kernel
    call kernel_main

    ; If kernel returns, hang the CPU (infinite loop)
    cli
.hang:	hlt
    jmp .hang
.end:
