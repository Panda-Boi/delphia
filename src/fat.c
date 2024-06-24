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

uint32_t cluster_to_lba(uint16_t cluster, DISK* disk);
uint16_t next_cluster(uint16_t current_cluster);

DISK* current_disk;
FAT12_HEADER* header;
ROOT_DIR_ENTRY* root_dir_entries;
uint8_t* file_allocation_table;

size_t initialize_fat(void* address, DISK* disk) {

    // set initialized disk
    current_disk = disk;

    // reading the fat header into memory
    header = address;
    address += sizeof(FAT12_HEADER);

    DISK_ReadSectors(disk, 0, 1, header);

    // reading the root directory into memory
    size_t root_dir_sector = header->reserved_sectors + (header->fat_count * header->sectors_per_fat);
    size_t root_dir_sectors_count = (header->dir_entries_count * 32) / header->bytes_per_sector; // size of entry = 32 bytes, 512 bytes per sector

    root_dir_entries = address;
    address += sizeof(ROOT_DIR_ENTRY) * header->dir_entries_count;

    DISK_ReadSectors(disk, root_dir_sector, root_dir_sectors_count, root_dir_entries);

    // reading the FAT into memory
    file_allocation_table = address;
    address += header->sectors_per_fat * header->bytes_per_sector;

    size_t fat_sector = header->reserved_sectors;

    DISK_ReadSectors(disk, fat_sector, header->sectors_per_fat, file_allocation_table);

    size_t fat_metadata_size = ((void*)file_allocation_table - (void*)header) + (header->sectors_per_fat * header->bytes_per_sector);

    return fat_metadata_size;
}

bool file_read(char* file_name, void* address) {

    ROOT_DIR_ENTRY* file = file_find(file_name);

    size_t clusters_count = (file->size + header->bytes_per_sector - 1) / header->bytes_per_sector;
    clusters_count /= header->sectors_per_cluster;
    
    uint16_t current_cluster = file->first_cluster_low;

    for (int i=0;i<clusters_count;i++) {

        uint32_t cluster_lba = cluster_to_lba(current_cluster, current_disk);

        DISK_ReadSectors(current_disk, cluster_lba, header->sectors_per_cluster, address);

        current_cluster = next_cluster(current_cluster);
        address += header->bytes_per_sector * header->sectors_per_cluster;

    }


}

ROOT_DIR_ENTRY* file_find(char* file_name) {

    file_name = to_upper(file_name);

    for (int i=0;i<header->dir_entries_count;i++) {

        // create null terminated string from root dir entry
        char name[12];
        strcpy(root_dir_entries[i].name, name, 11);
        name[11] = '\0';

        if (strcmp(name, file_name)) {
            return &root_dir_entries[i];
        }
    }

    // file not found
    return NULL;

}

uint16_t next_cluster(uint16_t current_cluster) {

    size_t byte_offset = current_cluster / 2 + current_cluster;
    uint16_t cluster_entry = *(file_allocation_table + byte_offset);

    if (current_cluster % 2 == 0) {
        // even cluster
        // return only bottom 12 bits out of 16
        return (cluster_entry & 0x0FFF);
    } else {
        // odd cluster
        // return only top 12 bits out of 16
        return (cluster_entry >> 4);
    }

}

uint32_t cluster_to_lba(uint16_t cluster, DISK* disk) {

    uint32_t data_sector = header->reserved_sectors // reserved sectors
                        + (header->fat_count * header->sectors_per_fat) // fat sectors
                        + (header->dir_entries_count * 32) / header->bytes_per_sector; // root dir sectors
    
    return data_sector + cluster - 2;

}