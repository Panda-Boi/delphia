[bits 32]

section .text

global _start
_start:
    [extern main]

    mov al, 'X'
    mov ah, 0x0f
    mov [0xb8004], ax   ; write Q at video memory

    call main

    jmp $
