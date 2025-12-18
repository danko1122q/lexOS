
bits 32

; Multiboot header
section .multiboot
align 4
    dd 0x1BADB002            ; Magic number
    dd 0x00                   ; Flags
    dd -(0x1BADB002 + 0x00)  ; Checksum

section .text.entry
global kernel_entry
extern kernel_main

kernel_entry:
    ; Set up stack
    mov esp, kernel_stack_top
    
    ; Push multiboot info
    push ebx    ; Multiboot info structure
    push eax    ; Multiboot magic number
    
    ; Call kernel main
    call kernel_main
    
    ; Hang if kernel returns
.hang:
    cli
    hlt
    jmp .hang

section .bss
align 16
kernel_stack_bottom:
    resb 16384  ; 16 KB stack
kernel_stack_top:
