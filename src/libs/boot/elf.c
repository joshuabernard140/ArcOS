#include "elf.h"
#include "fat.h"
#include "memory.h"
#include "stdio.h"
#include "../util/memdefs.h"
#include "../util/minmax.h"

bool ELF_Read(Partition* part, const char* path, void** entryPoint) {
    uint8_t* headerBuffer = MEMORY_ELF_ADDR;
    uint8_t* loadBuffer = MEMORY_LOAD_KERNEL;
    uint32_t filePosition = 0;
    uint32_t read;

    //Read header
    FAT_File* fd = FAT_Open(part, path);
    if ((read = FAT_Read(part, fd, sizeof(ELFHeader), headerBuffer)) != sizeof(ELFHeader)) {
        printf("ELF Load error!\n");
        return false;
    }
    filePosition += read;

    //Validate header
    bool ok = true;
    ELFHeader* header = (ELFHeader*)headerBuffer;
    ok = ok && (memcmp(header->Magic, ELF_MAGIC, 4) != 0);
    ok = ok && (header->Bitness == ELF_BITNESS_32BIT);
    ok = ok && (header->Endianness == ELF_ENDIANNESS_LITTLE);
    ok = ok && (header->ELFHeaderVersion == 1);
    ok = ok && (header->ELFVersion == 1);
    ok = ok && (header->Type == ELF_TYPE_EXECUTABLE);
    ok = ok && (header->InstructionSet == ELF_INSTRUCTION_SET_X86);

    *entryPoint = (void*)header->ProgramEntryPosition;

    //Load program header
    uint32_t programHeaderOffset = header->ProgramHeaderTablePosition;
    uint32_t programHeaderSize = header->ProgramHeaderTableEntrySize * header->ProgramHeaderTableEntryCount;
    uint32_t programHeaderTableEntrySize = header->ProgramHeaderTableEntrySize;
    uint32_t programHeaderTableEntryCount = header->ProgramHeaderTableEntryCount;

    filePosition += FAT_Read(part, fd, programHeaderOffset - filePosition, headerBuffer);
    if ((read = FAT_Read(part, fd, programHeaderSize, headerBuffer)) != programHeaderSize) {
        printf("ELF Load error\n");
        return false;
    }
    filePosition += read;
    FAT_Close(fd);

    //Parse program header entries
    for (uint32_t i = 0; i < programHeaderTableEntryCount; i++) {
        ELFProgramHeader* programHeader = (ELFProgramHeader*)(headerBuffer + i * programHeaderTableEntrySize);
        if (programHeader->Type == ELF_PROGRAM_TYPE_LOAD) {
            uint8_t* virtualAddress = (uint8_t*)programHeader->VirtualAddress;
            memset(virtualAddress, 0, programHeader->MemorySize);
            
            //Seeking
            fd = FAT_Open(part, path);
            while (programHeader->Offset > 0) {
                uint32_t shouldRead = min(programHeader->Offset, MEMORY_LOAD_SIZE);
                read = FAT_Read(part, fd, shouldRead, loadBuffer);
                if (read != shouldRead) {
                    printf("ELF Load error\n");
                    return false;
                }
                programHeader->Offset -= read;
            }

            //Read program
            while (programHeader->FileSize > 0) {
                uint32_t shouldRead = min(programHeader->FileSize, MEMORY_LOAD_SIZE);
                read = FAT_Read(part, fd, shouldRead, loadBuffer);
                if (read != shouldRead) {
                    printf("ELF Load error\n");
                    return false;
                }
                programHeader->FileSize -= read;
                memcpy(virtualAddress, loadBuffer, read);
                virtualAddress += read;
            }

            FAT_Close(fd);
        }
    }

    return true;
}