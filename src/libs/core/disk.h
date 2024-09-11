#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "../boot/disk.h"

bool Kernel_DISK_Initialize(DISK* disk, uint8_t driveNumber);
bool Floppy_Initialize(DISK* disk);
bool Hard_DISK_Initialize(DISK* disk);
bool Floppy_Read(DISK* disk, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t sectors, void* buffer);
bool Disk_Read(DISK* disk, uint32_t lba, uint8_t sectors, void* buffer, bool isIdentify);
bool Kernel_DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, void* dataOut, bool isIdentify);