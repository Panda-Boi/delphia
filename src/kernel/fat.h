#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "disk.h"

typedef struct {

    uint8_t name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t created_time_tenths;
    uint16_t created_time;
    uint16_t created_date;
    uint16_t accessed_date;
    uint16_t first_cluster_high;
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t first_cluster_low; 
    uint32_t size;

} __attribute__((packed)) DIR_ENTRY;

// 8.3 dir entry attribute constants
#define READ_ONLY   0x01
#define HIDDEN      0x02
#define SYSTEM      0x04
#define VOLUME_ID   0x08
#define DIRECTORY   0x10
#define ARCHIVE     0x20

/* Loads all the meta-data of a fat formatted disk at the given address and sets the disk to the currently open disk 
Returns the number of bytes loaded at the address given */
size_t initialize_fat(void* address, DISK* disk);

/* Finds the file with the given name in the root directory and returns its properties */
DIR_ENTRY* file_find(char* file_name);

/* Finds the file with the given id in the root directory and returns its properties */
DIR_ENTRY* file_id(size_t id);

/* Reads a file with the given name into memory at the address given
Ensure there is enough space at the address to fit the entire file
Returns true if succesful or false if failed */
bool file_read(char* file_name, void* address);

/* Writes a file at the address given with the given size and with the given name
Returns true if succesful or false if failed */
bool file_write(char* file_name, void* address, size_t len_bytes);