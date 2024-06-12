# Project Delphia
An OS written entirely from scratch
>Including the bootloader

## Currently Working
- Basic Bootloader that sets up the GDT and enters protected mode
- Basic Kernel written in C with a ***very*** limited terminal

## TODO
- Implementing Terminal Scrolling
- Keyboard Handling
- Global Constructors
- The beginnings of a Standard Library

## Running
Building the OS requires a C Cross-Compiler. Building one is way too tedious of a process to be included here currently but perhaps will be in the future.

For now just download the latest Binary from Releases. QEMU is being used currently for development purposes and so the same is recommended. However theoretically anything else such as virtualbox should work.

To run using QEMU, install it and execute:  
`qemu-system-x86_64 -drive format=raw,file="OS.bin",index=0,if=floppy,  -m 128M` 
