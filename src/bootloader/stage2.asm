[org 0x7e00]
[bits 16]

entry:
    jmp enter_protected_mode

halt:
    cli
    hlt

enter_protected_mode:

mov ah, 0x0
mov al, 0x3
int 0x10 ; switching to text mode

CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start

cli ; disable all interrupts
lgdt [GDT_descriptor] ; load gdt
; change last bit of cr0 to 1
mov eax, cr0
or eax, 1
mov cr0, eax

; CPU now in 32-bit mode
jmp CODE_SEG : start_protected_mode

GDT_start:
    GDT_null:
        dd 0x0 ; four times 00000000
        dd 0x0 ; four times 00000000

    ; base: 0 (32 bit)
    ; limit: 0xfffff (20 bit)
    ; pres, priv, type = 1001
    ; type flags = 1010
    ; other flags = 1100
    GDT_code:
        dw 0xffff
        ; first 4 bits of the limit 
        dw 0x0
        db 0x0
        ; first 24 bits of the base
        db 0b10011010
        ; pres, priv, type, type flags
        db 0b11001111
        ; other flags, last 4 bits of the limit
        db 0x0
        ; last 8 bits of the base

    ; base: 0 (32 bit)
    ; limit: 0xfffff (20 bit)
    ; pres, priv, type = 1001
    ; type flags = 0010
    ; other flags = 1100   
    GDT_data:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10010010
        db 0b11001111
        db 0x0

    GDT_16bit:
        ; 16-bit code segment
        dw 0FFFFh                   ; limit (bits 0-15) = 0xFFFFF
        dw 0                        ; base (bits 0-15) = 0x0
        db 0                        ; base (bits 16-23)
        db 10011010b                ; access (present, ring 0, code segment, executable, direction 0, readable)
        db 00001111b                ; granularity (1b pages, 16-bit pmode) + limit (bits 16-19)
        db 0                        ; base high

        ; 16-bit data segment
        dw 0FFFFh                   ; limit (bits 0-15) = 0xFFFFF
        dw 0                        ; base (bits 0-15) = 0x0
        db 0                        ; base (bits 16-23)
        db 10010010b                ; access (present, ring 0, data segment, executable, direction 0, writable)
        db 00001111b                ; granularity (1b pages, 16-bit pmode) + limit (bits 16-19)
        db 0                        ; base high

GDT_end:

GDT_descriptor:
    dw GDT_end - GDT_start - 1  ; size
    dd GDT_start                ; start

[bits 32]
start_protected_mode:
    mov ax, DATA_SEG    ; setup segment registers and stack
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0xffff    ; 32 bit stack base pointer
    mov esp, ebp

    jmp KERNEL_LOCATION ; jump to kernel location

KERNEL_LOCATION equ 0x8000
times 512 - ($ - $$) db 0