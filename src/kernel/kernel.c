#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "terminal.h"
#include "fat.h"
#include "disk.h"
#include "memory.h"
#include "string.h"
#include "shell.h"
#include "keyboard.h"

#include "gdt.h"
#include "idt.h"

#define BOOT_DRIVE 0

void* buffer = (void*) MEM_BUFFER;

extern void main(){

    i686_GDT_Initialize();
    //i686_IDT_Initialize();    

    terminal_initialize();

    print("Initializing Disk...");
    DISK disk;
    if (!DISK_Initialize(&disk, BOOT_DRIVE)) {
        print("Couldn't initialize disk ");
        print_int(BOOT_DRIVE);
        print("...");
    } else {

        size_t fat_size = initialize_fat(buffer, &disk);

        if (!fat_size) {
            print("Couldn't initialize fat...\n");
        } else {

            buffer += fat_size;

            print(" Done\n");
        }

    }

    print("Press any key to enter the shell...");
    keyboard_getinput();

    initialize_shell(buffer, disk);

    print("\nFinally decided to leave eh?");

    return;

}