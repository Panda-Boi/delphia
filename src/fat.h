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

} __attribute__((packed)) ROOT_DIR_ENTRY;

/* Loads all the meta-data of a fat formatted disk at the given address and sets the disk to the currently open disk 
Returns the number of bytes loaded at the address given */
size_t initialize_fat(void* address, DISK* disk);

/* Finds the file with the given name in the root directory and returns its properties */
ROOT_DIR_ENTRY* file_find(char* file_name);

/* Reads a file with the given name into memory at the address given
Ensure there is enough space at the address to fit the entire file
Returns true if succesful or false if failed */
bool file_read(char* file_name, void* address);
bool file_write(char* file_name, void* address);