[org 0x7c00]
KERNEL_LOCATION equ 0x1000

; bios loads the boot disk no into dl before jumping to 0x7C00
mov [BOOT_DISK], dl

; floppies => 512 bytes sectors, 18 sectors/track, 160 tracks
; reading from boot disk
; AH = 0x02
; AL = no of sectors to read
; CH = track / cylinder no
; CL = sector no
; DH = head no
; DL = drive no
; ES:BX = pointer to biff
xor ax, ax
mov es, ax
mov ds, ax
mov bp, 0x8000
mov sp, bp

mov bx, KERNEL_LOCATION
mov dh, 40 ; if something is broken, this number is probably too low
          ; no of sectors being read

mov ah, 0x02
mov al, dh 
mov ch, 0x00
mov dh, 0x00
mov cl, 0x02
mov dl, [BOOT_DISK]
int 0x13    ; no error management, do your homework!

mov ah, 0x0
mov al, 0x3
int 0x10 ; switching to text mode

CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start
    ; equ is used to set constants

cli ; disable all interrupts
lgdt [GDT_descriptor] ; load gdt
; change last bit of cr0 to 1
mov eax, cr0
or eax, 1
mov cr0, eax

; CPU now in 32-bit mode
jmp CODE_SEG : start_protected_mode

jmp $

BOOT_DISK: db 0

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

    mov ebp, 0x90000    ; 32 bit stack base pointer
    mov esp, ebp

    jmp KERNEL_LOCATION ; jump to kernel location

times 510 - ($ - $$) db 0
dw 0xaa55
