#include "fat.h"
#include "terminal.h"
#include "disk.h"
#include "memory.h"

typedef struct {
    //FAT12 Header
    uint8_t shortJump[3];
    uint8_t oem_indentifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t dir_entries_count;
    uint16_t total_sectors;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t large_sector_count;

    // Extended Boot Record
    uint8_t drive_number;
    uint8_t reserved_byte;
    uint8_t signature;
    uint8_t volume_id[4];
    uint8_t volume_label[11];
    uint8_t system_id[8];

    // Stage1 Bootloader Code

} __attribute__((packed)) FAT12_HEADER;


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

FAT12_HEADER* header = (FAT12_HEADER*) MEM_BUFFER;

bool initialize_fat(DISK* disk) {

    DISK_ReadSectors(disk, 0, 1, header);

    // print the name of the drive
    char label[12];
    for (int i=0;i<11;i++) {

        label[i] = header->volume_label[i];
        if (label[i] == ' ') {
            label[i] = '\0';
            break;
        }

    }
    print(label);
    print(":\\>FILES ON DISK:\n");

    size_t fat_sector = header->reserved_sectors;
    size_t root_dir_sector = header->reserved_sectors + (header->fat_count * header->sectors_per_fat);

    size_t root_dir_sectors_count = (header->dir_entries_count * 32) / 512; // size of entry = 32 bytes, 512 bytes per sector

    ROOT_DIR_ENTRY dir_entries[header->dir_entries_count];
    DISK_ReadSectors(disk, root_dir_sector, root_dir_sectors_count, dir_entries);
    
    for (int n=1;n<3;n++) {
        char file_name[12];
        for (int i=0;i<11;i++) {
            file_name[i] = dir_entries[n].name[i];
        }
        file_name[11] = '\0'; 
        print(file_name);
        print("\n");
    }

    return true;
}

bool file_load(char* file_name, void* address) {



}

size_t file_find(char* file_name, size_t drive_no) {

    return 1;
}