#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Struct containing all the properties of a particular disk
Drive No
No of cylinders
No of sectors per cylinder
No of heads */
typedef struct {
    uint8_t id;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
} DISK;

/* Input: Pointer to a disk struct and the drive no of the drive you want to initialize

Output: Initializes the disk struct with the properties of the given drive no; returns false if failed */
bool DISK_Initialize(DISK* disk, uint8_t driveNumber);

/* Input: Pointer to the disk struct of the disk you want to read, the lba address, no of sectors to be read, pointer to the memory address to read the data into

Output: Reads the required sectors into memory starting at the address given; returns false if failed */
bool DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, void* lowerDataOut);