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

#define BOOT_DRIVE 0

void* buffer = (void*) MEM_BUFFER;

extern void main(){

    terminal_initialize();

    print("Initializing Disk...");
    DISK disk;
    if (!DISK_Initialize(&disk, BOOT_DRIVE)) {
        print("Couldn't initialize disk ");
        print_int(BOOT_DRIVE);
        print("...");
    } else {
        print(" Done\n");
    }

    print("Press any key to enter the shell...");
    keyboard_getinput();

    initialize_shell(buffer, disk);

    print("\nFinally decided to leave eh?");

    return;

}