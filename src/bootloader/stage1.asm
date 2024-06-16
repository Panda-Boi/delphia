[org 0x7c00]
[bits 16]

; FAT12 Header
jmp short start
nop

bdb_oem:                    db "MSWIN4.1"   ; 8 bytes
bdb_bytes_per_sector:       dw 512          
bdb_sectors_per_cluster:    db 1
bdb_reserved_sectors:       dw 2
bdb_fat_count:              db 2
bdb_dir_entries_count:      dw 0x0e0
bdb_total_sectors:          dw 2880         ; 2880 * 512 bytes = 1440 Kib
bdb_media_descriptor_type:  db 0x0f0        ; f0 -> 3.5" floppy disk
bdb_sectors_per_fat:        dw 9
bdb_sectors_per_track:      dw 18
bdb_heads:                  dw 2
bdb_hidden_sectors:         dd 0
bdb_large_sector_count:     dd 0

; Extended Boot Record
ebr_drive_number:           db 0            ; 0x0 -> floppy1, 0x1 -> floppy2, 0x80 -> hard disk
                            db 0            ; reserved
ebr_signature:              db 0x29
ebr_volume_id:              db 0x10, 0x20, 0x30, 0x40   ; serial number
ebr_volume_label:           db "DELPHIA    "    ; 11 bytes padded with spaces
ebr_system_id:              db "FAT12   "       ; 8 bytes

; End of Header

start:
    ; setup data segments
    mov ax, 0           ; can't set ds/es directly
    mov ds, ax
    mov es, ax
    
    ; setup stack
    mov ss, ax
    mov sp, 0x7C00              ; stack grows downwards from where we are loaded in memory


    ; bios loads the boot disk no into dl before jumping to 0x7C00
    mov [ebr_drive_number], dl

    mov ax, 1       ; LBA = 1 (second sector of the disk)
    mov cl, 1       ; no of sectors to be read = 1
    mov bx, STAGE_2_LOCATION  ; read data into memory right after the bootloader
    call disk_read

    mov si, msg_loading
    call print

    push print      ; push memory location of the print function to the stack
    jmp STAGE_2_LOCATION

%define ENDL 0x0D, 0x0A
msg_loading:              db "Loading Stage 2...", ENDL, 0
msg_floppy_error:       db "Read from disk failed!", ENDL, 0

STAGE_2_LOCATION equ 0x7e00

; Prints a string to the screen
; Params:
;   - ds:si points to string
print:
    ; save registers we will modify
    push si
    push ax
    push bx

.loop:
    lodsb               ; loads next character in al
    or al, al           ; verify if next character is null?
    jz .done

    mov ah, 0x0E        ; call bios interrupt
    mov bh, 0           ; set page number to 0
    int 0x10

    jmp .loop

.done:
    pop bx
    pop ax
    pop si    
    ret

; Error Handlers
floppy_error:
    mov si, msg_floppy_error
    call print
    jmp wait_key_and_reboot

wait_key_and_reboot:
    mov ah, 0
    int 16h                     ; wait for keypress
    jmp 0FFFFh:0                ; jump to beginning of BIOS, should reboot

halt:
    cli                         ; disable interrupts, this way CPU can't get out of "halt" state
    hlt

; |=============|
; |Disk routines|
; |=============|

; Converts an LBA address to a CHS address
; Parameters:
;   - ax: LBA address
; Returns:
;   - cx [bits 0-5]: sector number
;   - cx [bits 6-15]: cylinder
;   - dh: head
lba_to_chs:

    push ax
    push dx

    xor dx, dx                          ; dx = 0
    div word [bdb_sectors_per_track]    ; ax = LBA / SectorsPerTrack
                                        ; dx = LBA % SectorsPerTrack

    inc dx                              ; dx = (LBA % SectorsPerTrack + 1) = sector
    mov cx, dx                          ; cx = sector

    xor dx, dx                          ; dx = 0
    div word [bdb_heads]                ; ax = (LBA / SectorsPerTrack) / Heads = cylinder
                                        ; dx = (LBA / SectorsPerTrack) % Heads = head
    mov dh, dl                          ; dh = head
    mov ch, al                          ; ch = cylinder (lower 8 bits)
    shl ah, 6
    or cl, ah                           ; put upper 2 bits of cylinder in CL

    pop ax
    mov dl, al                          ; restore dl
    pop ax
    ret

; Reads sectors from a disk
; Parameters:
;   - ax: LBA address
;   - cl: number of sectors to read (up to 128)
;   - dl: drive number
;   - es:bx: memory address where to store read data
disk_read:

    push ax                             ; save registers we will modify
    push bx
    push cx
    push dx
    push di

    push cx                             ; temporarily save CL (number of sectors to read)
    call lba_to_chs                     ; compute CHS
    pop ax                              ; AL = number of sectors to read
    
    mov ah, 02h
    mov di, 3                           ; retry count

.retry:
    pusha                               ; save all registers, we don't know what bios modifies
    stc                                 ; set carry flag, some BIOS'es don't set it
    int 13h                             ; carry flag cleared = success
    jnc .done                           ; jump if carry not set

    ; read failed
    popa
    call disk_reset

    dec di
    test di, di
    jnz .retry

.fail:
    ; all attempts are exhausted
    jmp floppy_error

.done:
    popa

    pop di
    pop dx
    pop cx
    pop bx
    pop ax                             ; restore registers modified
    ret

; Resets disk controller
; Parameters:
;   dl: drive number
disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc floppy_error
    popa
    ret

boot_sector_end:
    times 510 - ($ - $$) db 0
    dw 0xaa55