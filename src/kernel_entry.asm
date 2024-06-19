[bits 32]

section .text

global _start
_start:
    [extern main]

    call main

    jmp $
