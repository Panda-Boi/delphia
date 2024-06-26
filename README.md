# Project Delphia
An OS written entirely from scratch (for some reason)
>Including the bootloader (for some reason)

## Currently Working
- Bootloader that sets up the GDT and enters protected mode
- Basic Kernel written in C
- FAT12 Support (Read Only for now)
- Basic Shell

## TODO
- Interrupts
- Memory Manager

## Files
The floppy.img file that is created on running make is formatted with the FAT12 file system (the only one supported at the moment). mcopy can be used to write files to the image if you want to access them from inside Delphia.

## Building
Build the project using:
`make`
Clean the build directory using:
`make clean`

## Running
Building the OS requires a C Cross-Compiler. Building one is way too tedious of a process to be included here currently but perhaps will be in the future.

For now just download the latest Binary from Releases. QEMU is being used currently for development purposes and so the same is recommended. However theoretically anything else such as virtualbox should work.

To run using QEMU, install it and execute:  
`qemu-system-x86_64 -fda build/floppy.img` 
