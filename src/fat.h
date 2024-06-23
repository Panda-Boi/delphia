#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "disk.h"

bool initialize_fat(DISK* disk);
bool file_load(char* file_name, void* address);
size_t file_find(char* file_name, size_t drive_no);
//bool fat_load(size_t drive_no, void* address);
//bool cluster_read(size_t cluster_no, void* address);
//bool cluster_write(size_t cluster_no, void* address);