#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "terminal.h"
#include "fat.h"
#include "disk.h"
#include "memory.h"

#define BOOT_DRIVE 0

void* buffer = (void*) MEM_BUFFER;

extern void main(){

    terminal_initialize();

    DISK disk;
    DISK_Initialize(&disk, BOOT_DRIVE);

    void* fatAddress = buffer;
    buffer += 512;
    initialize_fat(&disk);

    return;

}