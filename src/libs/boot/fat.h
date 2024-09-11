#pragma once
#include <stddef.h>
#include <stdint.h>
#include "mbr.h"

#define SECTOR_SIZE            512
#define MAX_PATH_SIZE          256
#define MAX_FILE_HANDLES       10
#define ROOT_DIRECTORY_HANDLE  -1
#define FAT_CACHE_SIZE         5

typedef struct {
    uint8_t Name[11];
    uint8_t Attributes;
    uint8_t _Reserved;
    uint8_t CreatedTimeTenths;
    uint16_t CreatedTime;
    uint16_t CreatedDate;
    uint16_t AccessedDate;
    uint16_t FirstClusterHigh;
    uint16_t ModifiedTime;
    uint16_t ModifiedDate;
    uint16_t FirstClusterLow;
    uint32_t Size;
} __attribute__((packed)) FAT_DirectoryEntry;

typedef struct {
    uint8_t Order;
    int16_t Chars1[5];
    uint8_t Attribute;
    uint8_t LongEntryType;
    uint8_t Checksum;
    int16_t Chars2[6];
    uint16_t _AlwaysZero;
    int16_t Chars3[2];
} __attribute__((packed)) FAT_LongFileEntry;

#define FAT_LFN_LAST 0x40
                                    
typedef struct {
    int Handle;
    bool IsDirectory;
    uint32_t Position;
    uint32_t Size;
} FAT_File;

enum FAT_Attributes {
    FAT_ATTRIBUTE_READ_ONLY  = 0x01,
    FAT_ATTRIBUTE_HIDDEN     = 0x02,
    FAT_ATTRIBUTE_SYSTEM     = 0x04,
    FAT_ATTRIBUTE_VOLUME_ID  = 0x08,
    FAT_ATTRIBUTE_DIRECTORY  = 0x10,
    FAT_ATTRIBUTE_ARCHIVE    = 0x20,
    FAT_ATTRIBUTE_LFN        = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

typedef struct {
    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;
    uint8_t VolumeLabel[11];
    uint8_t SystemId[8];
} __attribute__((packed)) FAT_ExtendedBootRecord;

typedef struct {
    uint32_t SectorsPerFat;
    uint16_t Flags;
    uint16_t FatVersion;
    uint32_t RootDirectoryCluster;
    uint16_t FSInfoSector;
    uint16_t BackupBootSector;
    uint8_t _Reserved[12];
    FAT_ExtendedBootRecord EBR;

} __attribute((packed)) FAT32_ExtendedBootRecord;

typedef struct {
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    union {
        FAT_ExtendedBootRecord EBR1216;
        FAT32_ExtendedBootRecord EBR32;
    };

} __attribute__((packed)) FAT_BootSector;


typedef struct {
    uint8_t Buffer[SECTOR_SIZE];
    FAT_File Public;
    bool Opened;
    uint32_t FirstCluster;
    uint32_t CurrentCluster;
    uint32_t CurrentSectorInCluster;

} FAT_FileData;

typedef struct {
    uint8_t Order;
    int16_t Chars[13];
} FAT_LFNBlock;

typedef struct {
    union {
        FAT_BootSector BootSector;
        uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;

    FAT_FileData RootDirectory;
    FAT_FileData OpenedFiles[MAX_FILE_HANDLES];
    uint8_t FatCache[FAT_CACHE_SIZE * SECTOR_SIZE];
    uint32_t FatCachePosition;
    FAT_LFNBlock LFNBlocks[FAT_LFN_LAST];
    int LFNCount;

} FAT_Data;

int FAT_CompareLFNBlocks(const void* blockA, const void* blockB);
void FAT_Detect();
bool FAT_Initialize(Partition* part);
uint32_t FAT_ClusterToLba(uint32_t cluster);
FAT_File* FAT_OpenEntry(Partition* part, FAT_DirectoryEntry* entry);
uint32_t FAT_NextCluster(Partition* part, uint32_t currentCluster);
uint32_t FAT_Read(Partition* part, FAT_File* file, uint32_t byteCount, void* dataOut);
bool FAT_ReadEntry(Partition* part, FAT_File* file, FAT_DirectoryEntry* dirEntry);
void FAT_Close(FAT_File* file);
void FAT_GetShortName(const char* name, char shortName[12]);
bool FAT_FindFile(Partition* part, FAT_File* file, const char* name, FAT_DirectoryEntry* entryOut);
FAT_File * FAT_Open(Partition* part, const char* path);
void FAT_ReadDirectory(Partition* part, const char* path);
void FAT_ReadFile(Partition* part, const char* path);


