# Project Delphia
An OS written entirely from scratch
>Including the bootloader

## Currently Working
- Bootloader that sets up the GDT and enters protected mode
- Basic Kernel written in C with a ***very*** limited terminal
- FAT12 Support (Read Only for now)

## TODO
- Keyboard Handling
- Basic Shell
- Interrupts
- Memory Manager

## Running
Building the OS requires a C Cross-Compiler. Building one is way too tedious of a process to be included here currently but perhaps will be in the future.

For now just download the latest Binary from Releases. QEMU is being used currently for development purposes and so the same is recommended. However theoretically anything else such as virtualbox should work.

To run using QEMU, install it and execute:  
`qemu-system-x86_64 -fda build/floppy.img` 
