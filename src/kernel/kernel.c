#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "terminal.h"
#include "fat.h"
#include "disk.h"
#include "memory.h"
#include "string.h"
#include "keyboard.h"

#define BOOT_DRIVE 0

void* buffer = (void*) MEM_BUFFER;

extern void main(){

    terminal_initialize();

    DISK disk;
    DISK_Initialize(&disk, BOOT_DRIVE);

    void* fatAddress = buffer;
    buffer += initialize_fat(fatAddress, &disk);

    file_read("test    txt", buffer);

    while (true) {
        char c = keyboard_getinput();
        if (c) {
            char str[] = {0, 0};
            str[0] = c;
            print(str);
        }
    }

    return;

}