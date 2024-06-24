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

    ; load stage2 of the bootloader into memory
    mov ax, 1       ; LBA = 1 (second sector of the disk)
    mov cl, 1       ; no of sectors to be read = 1
    mov bx, STAGE_2_LOCATION  ; read data into memory right after the bootloader
    call disk_read

    ; some BIOSes might start us at 07C0:0000 instead of 0000:7C00, make sure we are in the
    ; expected location
    push es
    push word .after
    retf

.after:

    ; read something from floppy disk
    ; BIOS should set DL to drive number
    mov [ebr_drive_number], dl

    ; read drive parameters (sectors per track and head count),
    ; instead of relying on data on formatted disk
    push es
    mov ah, 08h
    int 13h
    jc floppy_error
    pop es

    and cl, 0x3F                        ; remove top 2 bits
    xor ch, ch
    mov [bdb_sectors_per_track], cx     ; sector count

    inc dh
    mov [bdb_heads], dh                 ; head count

    ; compute LBA of root directory = reserved + fats * sectors_per_fat
    ; note: this section can be hardcoded
    mov ax, [bdb_sectors_per_fat]
    mov bl, [bdb_fat_count]
    xor bh, bh
    mul bx                              ; ax = (fats * sectors_per_fat)
    add ax, [bdb_reserved_sectors]      ; ax = LBA of root directory
    push ax

    ; compute size of root directory = (32 * number_of_entries) / bytes_per_sector
    mov ax, [bdb_dir_entries_count]
    shl ax, 5                           ; ax *= 32
    xor dx, dx                          ; dx = 0
    div word [bdb_bytes_per_sector]     ; number of sectors we need to read

    test dx, dx                         ; if dx != 0, add 1
    jz .root_dir_after
    inc ax                              ; division remainder != 0, add 1
                                        ; this means we have a sector only partially filled with entries
.root_dir_after:

    ; read root directory
    mov cl, al                          ; cl = number of sectors to read = size of root directory
    pop ax                              ; ax = LBA of root directory
    mov dl, [ebr_drive_number]          ; dl = drive number (we saved it previously)
    mov bx, ROOT_DIR_BUFFER             ; es:bx = ROOT_DIR_BUFFER
    call disk_read

    ; search for kernel.bin
    xor bx, bx
    mov di, ROOT_DIR_BUFFER             ; di points to first file name

.search_kernel:
    mov si, file_kernel_bin             ; name of kernel file
    mov cx, 11                          ; compare up to 11 characters
    push di                             
    repe cmpsb
    pop di
    je .found_kernel

    ; move to next root dir entry
    add di, 32                          ; size of directory entry = 32
    inc bx
    cmp bx, [bdb_dir_entries_count]     ; check if no more dir entries are left
    jl .search_kernel

    ; kernel not found
    jmp kernel_not_found_error

.found_kernel:

    ; di should have the address to the entry
    mov ax, [di + 26]                   ; first logical cluster field (offset 26 in dir entry)
    mov [kernel_cluster], ax

    ; load FAT from disk into memory
    mov ax, [bdb_reserved_sectors]
    mov bx, FAT_BUFFER
    mov cl, [bdb_sectors_per_fat]
    mov dl, [ebr_drive_number]
    call disk_read

    ; read kernel and process FAT chain
    mov bx, KERNEL_LOAD_SEGMENT
    ;mov es, bx
    ;mov bx, KERNEL_LOAD_OFFSET

.load_kernel_loop:
    
    ; Read next cluster
    mov ax, [kernel_cluster]
    
    ; not nice :( hardcoded value
    add ax, 32                          ; first cluster = start_sector + (kernel_cluster - 2) * sectors_per_cluster
                                        ; start sector = reserved + fats + root directory size = 2 + 18 + 14 = 34
    mov cl, 1
    mov dl, [ebr_drive_number]
    call disk_read

    add bx, [bdb_bytes_per_sector]

    ; compute location of next cluster
    mov ax, [kernel_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx                              ; ax = index of entry in FAT, dx = cluster mod 2

    mov si, FAT_BUFFER
    add si, ax
    mov ax, [ds:si]                     ; read entry from FAT table at index ax

    or dx, dx
    jz .even

.odd:
    shr ax, 4
    jmp .next_cluster_after

.even:
    and ax, 0x0FFF

.next_cluster_after:
    cmp ax, 0x0FF8                      ; end of chain
    jae .read_finish

    mov [kernel_cluster], ax
    jmp .load_kernel_loop

.read_finish:
    
    mov dl, [ebr_drive_number]          ; boot device in dl

    jmp STAGE_2_LOCATION                ; jump to stage 2 of the bootloader

    jmp wait_key_and_reboot             ; should never happen

    cli                                 ; disable interrupts, this way CPU can't get out of "halt" state
    hlt

%define ENDL 0x0D, 0x0A
msg_floppy_error:       db "Read from disk failed", ENDL, 0
msg_kernel_not_found:   db "KERNEL.BIN not found", ENDL, 0
file_kernel_bin:        db "KERNEL  BIN"
kernel_cluster:         dw 0

ROOT_DIR_BUFFER equ 0x500           ; both are read into same area of memmory
FAT_BUFFER equ 0x500
STAGE_2_LOCATION equ 0x7e00         ; Memory right after the bootloader
KERNEL_LOAD_SEGMENT     equ 0x8000
KERNEL_LOAD_OFFSET      equ 0

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

kernel_not_found_error:
    mov si, msg_kernel_not_found
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