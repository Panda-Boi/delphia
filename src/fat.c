#include "fat.h"
#include "terminal.h"
#include "disk.h"

const size_t DRIVE_NO = 0;

bool initialize_fat(int8_t* address) {

    //load fat somewhere
    // where tho
    // hmmmmmm
    // i might load it in 0x500

    int8_t* fat_address = address;

    for (int i=0;i<20;i++) {
        print_int(*fat_address);
        print(" ");
    }

    return true;
}

bool file_load(char* file_name, void* address) {
    
    // try to find file "file_name" in drive 0 (update it to all drives perhaps)
    size_t first_cluster = file_find(file_name, DRIVE_NO);
    if (!first_cluster) {
        print("File not found...\n");
    }

    uint8_t* fat_address = (uint8_t*) 0x500;
    while (*fat_address) {
        print_int(*fat_address);
    }

    return true;
}

size_t file_find(char* file_name, size_t drive_no) {

    return 1;
}