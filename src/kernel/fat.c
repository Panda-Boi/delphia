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
    uint16_t root_dir_entries_count;
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
uint32_t fat_to_lba(size_t fat_index, DISK* disk);
uint16_t next_cluster(uint16_t current_cluster);
bool update_cluster(uint16_t cluster, uint16_t updated_value);
bool to_fat_fname(char* fname, char* fat_fname);

DISK* current_disk;
FAT12_HEADER* header;
DIR_ENTRY* root_dir_entries;
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
    size_t root_dir_sectors_count = (header->root_dir_entries_count * 32) / header->bytes_per_sector; // size of entry = 32 bytes, 512 bytes per sector

    root_dir_entries = address;
    address += sizeof(DIR_ENTRY) * header->root_dir_entries_count;

    DISK_ReadSectors(disk, root_dir_sector, root_dir_sectors_count, root_dir_entries);

    // reading the FAT into memory
    file_allocation_table = address;
    address += header->sectors_per_fat * header->bytes_per_sector;

    size_t fat_sector = header->reserved_sectors; // 0 + reserved sectors (2 sectors for the bootloader)

    DISK_ReadSectors(disk, fat_sector, header->sectors_per_fat, file_allocation_table);

    size_t fat_metadata_size = ((void*)address - (void*)header); // fat header + root dir + fat

    return fat_metadata_size;
}

bool file_read(char* file_name, void* address) {

    DIR_ENTRY* file = file_find(file_name);

    if (!file) {
        return false;
    }

    size_t clusters_count = (file->size + header->bytes_per_sector - 1) / header->bytes_per_sector;
    clusters_count /= header->sectors_per_cluster;
    
    uint16_t current_cluster = file->first_cluster_low;

    for (int i=0;i<clusters_count;i++) {

        uint32_t cluster_lba = cluster_to_lba(current_cluster, current_disk);

        DISK_ReadSectors(current_disk, cluster_lba, header->sectors_per_cluster, address);

        current_cluster = next_cluster(current_cluster);
        address += header->bytes_per_sector * header->sectors_per_cluster;

    }

    return true;

}

bool file_write(char* file_name, void* address, size_t len_bytes){

    if (strlen(file_name) > 11) {
        print("Given name is longer than 11 characters, file write failed...\n");
        return false;
    }

    // determine number of clusters
    size_t bytes_per_cluster = header->bytes_per_sector * header->sectors_per_cluster;
    size_t clusters_count = (len_bytes + bytes_per_cluster - 1) / bytes_per_cluster;
    size_t clusters_allocated[clusters_count + 1]; // last value is for end of chain signifier

    // check if file with given name already exists
    DIR_ENTRY* file = file_find(file_name);
    if (file) {
        // must update existing clusters
        uint16_t current_cluster = file->first_cluster_low;
        size_t i = 0;
        bool all_clusters_used = false;
        for (i=0;i<clusters_count;i++) {
            clusters_allocated[i] = current_cluster;
            current_cluster = next_cluster(current_cluster);

            // break if cluster chain is finished
            if (!current_cluster) {
                all_clusters_used = true;
                break;
            }
        }

        // if not all existing clusters will be used, set extra clusters as empty
        if (!all_clusters_used) {
            while(current_cluster) {
                uint16_t temp = next_cluster(current_cluster);
                update_cluster(current_cluster, 0);
                current_cluster = temp;
            }
        }

        // there were not enough clusters already allocated
        if (i < clusters_count) {
            // find empty clusters
            uint16_t current_cluster = 2;
            while(i < clusters_count) {
                // if next cluster is not 0, that means cluster is not empty]
                if (next_cluster(current_cluster)) {
                    current_cluster++;
                    continue;
                }
                
                // current cluster is empty
                clusters_allocated[i] = current_cluster;
                i++;
                current_cluster++;
            }
        }

    } else {
        // find empty clusters
        uint16_t current_cluster = 2;
        size_t i = 0;
        while(i < clusters_count) {
            // if next cluster is not 0, that means cluster is not empty]
            if (next_cluster(current_cluster)) {
                current_cluster++;
                continue;
            }

            // current cluster is empty
            clusters_allocated[i] = current_cluster;
            i++;
            current_cluster++;
        }
    }

    // end of chain signifier
    clusters_allocated[clusters_count] = 0xFF8;
    
    // write to the fat in memory
    for (int i=0;i<clusters_count;i++){
        update_cluster(clusters_allocated[i], clusters_allocated[i+1]);
    }

    // write to the root dir in memory

    // find a spot to write a new dir entry if one doesnt exist already
    // starting from 1 because id 0 is the volume info
    if (!file) {
        for (int i=1;i<header->root_dir_entries_count;i++) {
            DIR_ENTRY* entry = file_id(i);
    
            if (!entry->first_cluster_low) {
                file = entry;
                break;
            }
        }
    }

    // update file entry
    *file = (DIR_ENTRY) {
        .name = 0,
        .attributes = ARCHIVE,
        .reserved = 0,
        .created_time_tenths = 0,
        .created_time = 0,
        .created_date = 0,
        .accessed_date = 0,
        .first_cluster_high = 0,
        .modified_time = 0,
        .modified_date = 0,
        .first_cluster_low = clusters_allocated[0],
        .size = len_bytes
    };
    if (!to_fat_fname(to_upper(file_name), file->name)) {
        print("Given name was not a compatible Fat file name");
        to_fat_fname("UNDEF.BIN", file->name);
    }

    // write to the fats on disk
    for (int i=0;i<header->fat_count;i++) {
        uint32_t fat_lba = fat_to_lba(i, current_disk);
        DISK_WriteSectors(current_disk, fat_lba, header->sectors_per_fat, file_allocation_table);
    }

    // write to the root dir on disk
    size_t root_dir_sector = header->reserved_sectors + (header->fat_count * header->sectors_per_fat);
    size_t root_dir_sectors_count = (header->root_dir_entries_count * 32) / header->bytes_per_sector; // size of entry = 32 bytes, 512 bytes per sector
    DISK_WriteSectors(current_disk, root_dir_sector, root_dir_sectors_count, root_dir_entries);

    // write to the clusters
    for (int i=0;i<clusters_count;i++){
        
        uint32_t cluster_lba = cluster_to_lba(clusters_allocated[i], current_disk);
        size_t bytes = bytes_per_cluster ? i < clusters_count - 1 : len_bytes % bytes_per_cluster;

        DISK_WriteSectors(current_disk, cluster_lba, header->sectors_per_cluster, address);

        address += bytes_per_cluster;

    }

    return false;
}

bool file_delete(char* file_name) {

    if (strlen(file_name) > 11) {
        print("Given name is longer than 11 characters, file write failed...\n");
        return false;
    }

    // check if file with given name exists
    DIR_ENTRY* file = file_find(file_name);

    if (!file) {
        print("File with the given name does not exist\n");
        return false;
    }

    // update fat in memory
    uint16_t current_cluster = file->first_cluster_low;
    while(current_cluster) {
        uint16_t temp = next_cluster(current_cluster);
        update_cluster(current_cluster, 0);
        current_cluster = temp;
    }

    // update root dir in memory
    *file = (DIR_ENTRY) {
        .name = {0xE5, 0},
        0
    };

    // write to the fats on disk
    for (int i=0;i<header->fat_count;i++) {
        uint32_t fat_lba = fat_to_lba(i, current_disk);
        DISK_WriteSectors(current_disk, fat_lba, header->sectors_per_fat, file_allocation_table);
    }

    // write to the root dir on disk
    size_t root_dir_sector = header->reserved_sectors + (header->fat_count * header->sectors_per_fat);
    size_t root_dir_sectors_count = (header->root_dir_entries_count * 32) / header->bytes_per_sector; // size of entry = 32 bytes, 512 bytes per sector
    DISK_WriteSectors(current_disk, root_dir_sector, root_dir_sectors_count, root_dir_entries);

    return true;

}

DIR_ENTRY* file_find(char* file_name) {

    file_name = to_upper(file_name);
    char fat_fname[12];
    
    // incorrect file name, return null
    if (!to_fat_fname(file_name, fat_fname)) {
        return NULL;
    }

    for (int i=0;i<header->root_dir_entries_count;i++) {

        // create null terminated string from root dir entry
        char name[12];
        strcpy(root_dir_entries[i].name, name, 11);
        name[11] = '\0';

        if (strcmp(name, fat_fname)) {
            return &root_dir_entries[i];
        }
    }

    // file not found
    return NULL;

}

DIR_ENTRY* file_id(size_t id) {

    return &root_dir_entries[id];

}

// Returns 0 if there is no next cluster
uint16_t next_cluster(uint16_t cluster) {

    size_t byte_offset = cluster / 2 + cluster;
    uint16_t cluster_entry = *(uint16_t*)(file_allocation_table + byte_offset);

    uint16_t next_cluster;

    if (cluster % 2 == 0) {
        // even cluster
        // return only bottom 12 bits out of 16
        next_cluster = (cluster_entry & 0x0FFF);
    } else {
        // odd cluster
        // return only top 12 bits out of 16
        next_cluster = (cluster_entry >> 4);
    }

    // return 0 if next cluster is end of chain, i.e. no next cluster
    return 0 ? next_cluster >= 0xFF8 : next_cluster;
}

// Returns true if successfull
bool update_cluster(uint16_t cluster, uint16_t updated_value) {
    size_t byte_offset = cluster / 2 + cluster;
    uint16_t* cluster_entry = (uint16_t*)(file_allocation_table + byte_offset);

    if (cluster % 2 == 0) {
        // even cluster
        // write only bottom 12 bits out of 16
        *cluster_entry = (*cluster_entry & 0xF000) | updated_value;
    } else {
        // odd cluster
        // write only top 12 bits out of 16
        *cluster_entry = (*cluster_entry & 0x000F) | (updated_value << 4);
    }

    return true;
}

uint32_t cluster_to_lba(uint16_t cluster, DISK* disk) {

    uint32_t data_sector = header->reserved_sectors // reserved sectors
                        + (header->fat_count * header->sectors_per_fat) // fat sectors
                        + (header->root_dir_entries_count * 32) / header->bytes_per_sector; // root dir sectors
    
    return data_sector + cluster - 2;

}

uint32_t fat_to_lba(size_t fat_index, DISK* disk) {
    return header->reserved_sectors + (fat_index * header->sectors_per_fat);
}

bool to_fat_fname(char* fname, char* fat_fname) {

    size_t len = strlen(fname);

    size_t i = 0;
    while(fname[i] != '.') {
        if (i >= len || i > 8) {
            return false;
        }
        i++;
    }

    if (len - i != 4) {
        return false;
    }

    strcpy(fname, fat_fname, i);

    // move fname pointer to the file extension
    fname += i + 1;

    while (i < 8) {
        fat_fname[i] = ' ';
        i++;
    }

    strcpy(fname, (fat_fname + 8), 3);

    fat_fname[11] = '\0';

    return true;

}