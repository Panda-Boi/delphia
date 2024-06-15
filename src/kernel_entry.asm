[bits 32]

section .text

global _start
_start:
    [extern main]

    mov eax, 69
    push eax
    call main
    jmp $
