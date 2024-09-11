#pragma once
#include <stddef.h>
#include <stdint.h>
#include "disk.h"
#include "../boot/mbr.h"
#include "../boot/fat.h"

void Kernel_FAT_Detect();
bool Kernel_FAT_Initialize(Partition* part);
uint32_t Kernel_FAT_ClusterToLba(uint32_t cluster);
FAT_File* Kernel_FAT_OpenEntry(Partition* part, FAT_DirectoryEntry* entry);
uint32_t Kernel_FAT_NextCluster(Partition* part, uint32_t currentCluster);
uint32_t Kernel_FAT_Read(Partition* part, FAT_File* file, uint32_t byteCount, void* dataOut);
bool Kernel_FAT_ReadEntry(Partition* part, FAT_File* file, FAT_DirectoryEntry* dirEntry);
void Kernel_FAT_Close(FAT_File* file);
void Kernel_FAT_GetShortName(const char* name, char shortName[12]);
bool Kernel_FAT_FindFile(Partition* part, FAT_File* file, const char* name, FAT_DirectoryEntry* entryOut);
FAT_File * Kernel_FAT_Open(Partition* part, const char* path);
void Kernel_FAT_ReadDirectory(Partition* part, const char* path);
void Kernel_FAT_ReadFile(Partition* part, const char* path);