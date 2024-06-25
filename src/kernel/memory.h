// Manual memory section declarations
#define MEM_STACK 0xFFFF // grows downward

#define MEM_BOOTLOADER 0x7C00
#define MEM_KERNEL 0x8000

#define MEM_BUFFER 0x10000

/*
MEMORY MAP (LOWER MEMORY)

0x0 - 0x4FF = BIOS

0x500 - 0x7BFF = STACK

0x7C00 - 0x7FFF = BOOTLOADER

0x8000 - 0xFFFF = KERNEL

0x10000 - 0x7FFFF = BUFFER
*/
