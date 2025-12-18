# ============================================
# File: boot/stage1.asm - Simple Stage 1 Bootloader
# (For non-GRUB boot, optional)
# ============================================

; MiniOS v2.0 - Stage 1 Bootloader
; This is a simple bootloader for legacy BIOS boot
; GRUB is recommended for production use

[org 0x7C00]
[bits 16]

KERNEL_OFFSET equ 0x1000
KERNEL_SECTORS equ 30

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    ; Print loading message
    mov si, msg_loading
    call print_string
    
    ; Load kernel
    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x80
    mov bx, KERNEL_OFFSET
    int 0x13
    
    jc disk_error
    
    ; Switch to protected mode
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:protected_mode

disk_error:
    mov si, msg_error
    call print_string
    jmp $

print_string:
    pusha
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    popa
    ret

[bits 32]
protected_mode:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9FC00
    
    call KERNEL_OFFSET
    jmp $

[bits 16]

; GDT
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

gdt_code:
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

gdt_data:
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

msg_loading db "Loading LexOS v0.0.1...", 13, 10, 0
msg_error db "Disk read error!", 13, 10, 0

times 510-($-$$) db 0
dw 0xAA55
