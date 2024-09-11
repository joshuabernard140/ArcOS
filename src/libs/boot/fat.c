#include "fat.h"
#include "stdio.h"
#include "memory.h"
#include "../util/memdefs.h"
#include "../util/string.h"
#include "../util/ctype.h"
#include "../util/minmax.h"
#include "../util/stdlib.h"

static FAT_Data* g_Data;
static uint32_t g_DataSectionLba;
static uint8_t g_FatType;
static uint32_t g_TotalSectors;
static uint32_t g_SectorsPerFat;

int FAT_CompareLFNBlocks(const void* blockA, const void* blockB) {
    FAT_LFNBlock* a = (FAT_LFNBlock*)blockA;
    FAT_LFNBlock* b = (FAT_LFNBlock*)blockB;
    return ((int)a->Order) - ((int)b->Order);
}

void FAT_Detect() {
    uint32_t dataClusters = (g_TotalSectors - g_DataSectionLba) / g_Data->BS.BootSector.SectorsPerCluster;
    if (dataClusters < 0xFF5) {
        g_FatType = 12;
    } else if (g_Data->BS.BootSector.SectorsPerFat != 0) {
        g_FatType = 16;
    } else {
        g_FatType = 32;
    }
}

bool FAT_Initialize(Partition* part) {
    g_Data = (FAT_Data*)MEMORY_FAT_ADDR;

    if (!Partition_ReadSectors(part, 0, 1, g_Data->BS.BootSectorBytes)) {
        printf("FAT: read boot sector failed\r\n");
        return false;
    }

    g_Data->FatCachePosition = 0xFFFFFFFF;

    g_TotalSectors = g_Data->BS.BootSector.TotalSectors;
    if (g_TotalSectors == 0) {
        g_TotalSectors = g_Data->BS.BootSector.LargeSectorCount;
    }

    bool isFat32 = false;
    g_SectorsPerFat = g_Data->BS.BootSector.SectorsPerFat;
    if (g_SectorsPerFat == 0) {
        isFat32 = true;
        g_SectorsPerFat = g_Data->BS.BootSector.EBR32.SectorsPerFat;
    }

    uint32_t rootDirLba;
    uint32_t rootDirSize;
    if (isFat32) {
        g_DataSectionLba = g_Data->BS.BootSector.ReservedSectors + g_SectorsPerFat * g_Data->BS.BootSector.FatCount;
        rootDirLba = FAT_ClusterToLba( g_Data->BS.BootSector.EBR32.RootDirectoryCluster);
        rootDirSize = 0;
    }
    else {
        rootDirLba = g_Data->BS.BootSector.ReservedSectors + g_SectorsPerFat * g_Data->BS.BootSector.FatCount;
        rootDirSize = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
        uint32_t rootDirSectors = (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) / g_Data->BS.BootSector.BytesPerSector;
        g_DataSectionLba = rootDirLba + rootDirSectors;
    }

    g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
    g_Data->RootDirectory.Public.IsDirectory = true;
    g_Data->RootDirectory.Public.Position = 0;
    g_Data->RootDirectory.Public.Size = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
    g_Data->RootDirectory.Opened = true;
    g_Data->RootDirectory.FirstCluster = rootDirLba;
    g_Data->RootDirectory.CurrentCluster = rootDirLba;
    g_Data->RootDirectory.CurrentSectorInCluster = 0;

    if (!Partition_ReadSectors(part, rootDirLba, 1, g_Data->RootDirectory.Buffer)) {
        printf("FAT: read root directory failed\r\n");
        return false;
    }

    FAT_Detect();

    for (int i = 0; i < MAX_FILE_HANDLES; i++) {
        g_Data->OpenedFiles[i].Opened = false;
    }
    g_Data->LFNCount = 0;

    return true;
}

uint32_t FAT_ClusterToLba(uint32_t cluster) {
    return g_DataSectionLba + (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster;
}

FAT_File* FAT_OpenEntry(Partition* part, FAT_DirectoryEntry* entry) {
    int handle = -1;
    for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++) {
        if (!g_Data->OpenedFiles[i].Opened) {
            handle = i;
        }
    }

    if (handle < 0) {
        printf("FAT: out of file handles\r\n");
        return false;
    }

    FAT_FileData* fd = &g_Data->OpenedFiles[handle];
    fd->Public.Handle = handle;
    fd->Public.IsDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
    fd->Public.Position = 0;
    fd->Public.Size = entry->Size;
    fd->FirstCluster = entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
    fd->CurrentCluster = fd->FirstCluster;
    fd->CurrentSectorInCluster = 0;

    if (!Partition_ReadSectors(part, FAT_ClusterToLba(fd->CurrentCluster), 1, fd->Buffer)) {
        printf("FAT: open entry failed - read error cluster=%u lba=%u\n", fd->CurrentCluster, FAT_ClusterToLba(fd->CurrentCluster));
        for (int i = 0; i < 11; i++) {
            printf("%c", entry->Name[i]);
        }
        printf("\n");
        return false;
    }    

    fd->Opened = true;
    return &fd->Public;
}

uint32_t FAT_NextCluster(Partition* part, uint32_t currentCluster) {    
    uint32_t fatIndex;
    if (g_FatType == 12) {
        fatIndex = currentCluster * 3 / 2;
    } else if (g_FatType == 16) {
        fatIndex = currentCluster * 2;
    } else {
        fatIndex = currentCluster * 4;
    }

    uint32_t fatIndexSector = fatIndex / SECTOR_SIZE;
    if (fatIndexSector < g_Data->FatCachePosition || fatIndexSector >= g_Data->FatCachePosition + FAT_CACHE_SIZE) {
        Partition_ReadSectors(part, g_Data->BS.BootSector.ReservedSectors + fatIndexSector, FAT_CACHE_SIZE, g_Data->FatCache);
        g_Data->FatCachePosition = fatIndexSector;
    }

    fatIndex -= (g_Data->FatCachePosition * SECTOR_SIZE);

    uint32_t nextCluster;
    if (g_FatType == 12) {
        if (currentCluster % 2 == 0) {
            nextCluster = (*(uint16_t*)(g_Data->FatCache + fatIndex)) & 0x0FFF;
        } else {
            nextCluster = (*(uint16_t*)(g_Data->FatCache + fatIndex)) >> 4;
        }
        
        if (nextCluster >= 0xFF8) {
            nextCluster |= 0xFFFFF000;
        }
    } else if (g_FatType == 16) {
        nextCluster = *(uint16_t*)(g_Data->FatCache + fatIndex);
        if (nextCluster >= 0xFFF8) {
            nextCluster |= 0xFFFF0000;
        }
    } else {
        nextCluster = *(uint32_t*)(g_Data->FatCache + fatIndex);
    }

    return nextCluster;
}

uint32_t FAT_Read(Partition* part, FAT_File* file, uint32_t byteCount, void* dataOut) {
    FAT_FileData* fd = (file->Handle == ROOT_DIRECTORY_HANDLE) ? &g_Data->RootDirectory : &g_Data->OpenedFiles[file->Handle];
    uint8_t* u8DataOut = (uint8_t*)dataOut;

    if (!fd->Public.IsDirectory || (fd->Public.IsDirectory && fd->Public.Size != 0)) {
        byteCount = min(byteCount, fd->Public.Size - fd->Public.Position);
    }
   
    while (byteCount > 0) { 
        uint32_t leftInBuffer = SECTOR_SIZE - (fd->Public.Position % SECTOR_SIZE);
        uint32_t take = min(byteCount, leftInBuffer);

        memcpy(u8DataOut, fd->Buffer + fd->Public.Position % SECTOR_SIZE, take);
        u8DataOut += take;
        fd->Public.Position += take;
        byteCount -= take;

        if (leftInBuffer == take) {
            if (fd->Public.Handle == ROOT_DIRECTORY_HANDLE) {
                ++fd->CurrentCluster;
                if (!Partition_ReadSectors(part, fd->CurrentCluster, 1, fd->Buffer)) {
                    printf("FAT: read error\r\n");
                    break;
                }
            } else {
                if (++fd->CurrentSectorInCluster >= g_Data->BS.BootSector.SectorsPerCluster) {
                    fd->CurrentSectorInCluster = 0;
                    fd->CurrentCluster = FAT_NextCluster(part, fd->CurrentCluster);
                }

                if (fd->CurrentCluster >= 0xFFFFFFF8) {
                    fd->Public.Size = fd->Public.Position;
                    break;
                }

                if (!Partition_ReadSectors(part, FAT_ClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 1, fd->Buffer)) {
                    printf("FAT: read error\r\n");
                    break;
                }
            }
        }
    }

    return u8DataOut - (uint8_t*)dataOut;
}

bool FAT_ReadEntry(Partition* part, FAT_File* file, FAT_DirectoryEntry* dirEntry) {
    return FAT_Read(part, file, sizeof(FAT_DirectoryEntry), dirEntry) == sizeof(FAT_DirectoryEntry);
}

void FAT_Close(FAT_File* file) {
    if (file->Handle == ROOT_DIRECTORY_HANDLE) {
        file->Position = 0;
        g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
    } else {
        g_Data->OpenedFiles[file->Handle].Opened = false;
    }
}

void FAT_GetShortName(const char* name, char shortName[12]) {
    memset(shortName, ' ', 12);
    shortName[11] = '\0';

    const char* ext = strchr(name, '.');
    if (ext == NULL) {
        ext = name + 11;
    }

    for (int i = 0; i < 8 && name[i] && name + i < ext; i++) {
        shortName[i] = toupper(name[i]);
    }

    if (ext != name + 11) {
        for (int i = 0; i < 3 && ext[i + 1]; i++) {
            shortName[i + 8] = toupper(ext[i + 1]);
        }
    }
}

//TODO: Long File Name
bool FAT_FindFile(Partition* part, FAT_File* file, const char* name, FAT_DirectoryEntry* entryOut) {
    char shortName[12];
    //char longName[256];
    FAT_DirectoryEntry entry;

    FAT_GetShortName(name, shortName);

    while (FAT_ReadEntry(part, file, &entry)) {
        /*printf("entry: %u lfn: %u\n", entry.Attributes, FAT_ATTRIBUTE_LFN);
        if (entry.Attributes == FAT_ATTRIBUTE_LFN) {
            FAT_LongFileEntry* lfn = (FAT_LongFileEntry*)&entry;

            int index = g_Data->LFNCount++;
            g_Data->LFNBlocks[index].Order = lfn->Order & (FAT_LFN_LAST - 1);
            memcpy(g_Data->LFNBlocks[index].Chars, lfn->Chars1, sizeof(lfn->Chars1));
            memcpy(g_Data->LFNBlocks[index].Chars + 5, lfn->Chars2, sizeof(lfn->Chars2));
            memcpy(g_Data->LFNBlocks[index].Chars + 11, lfn->Chars1, sizeof(lfn->Chars3));

            if ((lfn->Order & FAT_LFN_LAST) != 0) {
                quicksort(g_Data->LFNBlocks, g_Data->LFNCount, sizeof(FAT_LFNBlock), FAT_CompareLFNBlocks);
                char* namePos = longName;
                for (int i = 0; i < g_Data->LFNCount; i++) {
                    int16_t* chars = g_Data->LFNBlocks[i].Chars;
                    int16_t* charsLimit = chars + 13;

                    while (chars < charsLimit && *chars != 0) {
                        int codepoint;
                        chars = utf16_to_codepoint(chars, &codepoint);
                        namePos = codepoint_to_utf8(codepoint, namePos);
                    }
                }
                *namePos = 0;
                printf("LFN: %s\n", longName);
            }
        }*/

        if (memcmp(shortName, entry.Name, 11) == 0) {
            *entryOut = entry;
            return true;
        }
    }
    
    return false;
}

FAT_File* FAT_Open(Partition* part, const char* path) {
    char name[MAX_PATH_SIZE];

    if (path[0] == '/') {
        path++;
    }
    
    FAT_File* current = &g_Data->RootDirectory.Public;
    while (*path) {
        bool isLast = false;
        const char* delim = strchr(path, '/');
        if (delim != NULL) {
            memcpy(name, path, delim - path);
            name[delim - path] = '\0';
            path = delim + 1;
        } else {
            unsigned len = strlen(path);
            memcpy(name, path, len);
            name[len + 1] = '\0';
            path += len;
            isLast = true;
        }

        FAT_DirectoryEntry entry;
        if (FAT_FindFile(part, current, name, &entry)) {
            FAT_Close(current);

            if (!isLast && entry.Attributes & FAT_ATTRIBUTE_DIRECTORY == 0) {
                printf("FAT: %s not a directory\r\n", name);
                return NULL;
            }

            current = FAT_OpenEntry(part, &entry);
        } else {
            FAT_Close(current);
            printf("FAT: %s not found\r\n", name);
            return NULL;
        }
    }

    return current;
}

void FAT_ReadDirectory(Partition* part, const char* path) {
    FAT_File* fd = FAT_Open(part, path);
    FAT_DirectoryEntry entry;
    int i = 0;
    printf("\n");
    while (FAT_ReadEntry(part, fd, &entry) && i++ < 7) {
        printf("  ");
        for (int i = 0; i < 11; i++) {
            putc(entry.Name[i]); 
        } 
        printf("\r\n");
    }
    FAT_Close(fd);
}

void FAT_ReadFile(Partition* part, const char* path) {
    char buffer[100];
    uint32_t read;
    FAT_File* fd = FAT_Open(part, path);
    while ((read = FAT_Read(part, fd, sizeof(buffer), buffer))) {
        for (uint32_t i = 0; i < read; i++) {
            if (buffer[i] == '\n') {
                putc('\r');
            }
            putc(buffer[i]);
        }
    }
    printf("\n");
    FAT_Close(fd);
}