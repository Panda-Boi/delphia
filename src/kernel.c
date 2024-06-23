#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "terminal.h"
#include "fat.h"
#include "disk.h"
#include "memory.h"

#define BOOT_DRIVE 0

extern void main(){

    terminal_initialize();
    print("Hello World!\n");

    DISK disk;
    DISK_Initialize(&disk, BOOT_DRIVE);
    
    print("Info: ");
    print_int(disk.cylinders);
    print("\n");

    void* dataAddress = (void*) MEM_BUFFER;
    DISK_ReadSectors(&disk, 0, 3, dataAddress);

    print("Done reading...\n");
  
    return;

}