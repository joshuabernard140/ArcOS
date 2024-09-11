#include <stdint.h>
#include "../../libs/boot/stdio.h"
#include "../../libs/boot/x86.h"
#include "../../libs/boot/disk.h"
#include "../../libs/boot/fat.h"
#include "../../libs/util/memdefs.h"
#include "../../libs/boot/memory.h"
#include "../../libs/boot/vbe.h"
#include "../../libs/boot/elf.h"
#include "../../libs/boot/mbr.h"
#include "../../libs/boot/bootparams.h"

uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL;
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;

BootParams g_BootParams;

typedef void (*KernelStart)(BootParams* bootParams);

void start(uint16_t bootDrive, void* partition) {
    clrscr();

    //Initialize DISK
    DISK disk;
    if (!DISK_Initialize(&disk, bootDrive)) {
        printf("Disk initialization error\r\n");
        goto end;
    }

    //Detect Partition
    Partition part;
    MBR_DetectPartition(&part, &disk, partition);

    //Initialize FAT
    if (!FAT_Initialize(&part)) {
        printf("FAT initialization error\r\n");
        goto end;
    }

    //Prepare Boot Params
    g_BootParams.BootDevice = bootDrive;
    mem_Detect(&g_BootParams.Memory);

    //Load Kernel
    KernelStart kernelEntry;
    if (!ELF_Read(&part, "/kernel.elf", (void**)&kernelEntry)) {
        printf("ELF read failed, booting halted\r\n");
        goto end;
    }

    printf("\nDISK: id=%u cylinders=%u heads=%u sectors=%u\n", disk.id, disk.cylinders, disk.heads, disk.sectors);
    printf("MBR: partitionOffset=%u partitionSize=%u\n", part.partitionOffset, part.partitionSize);

    FAT_ReadDirectory(&part, "/"); //Read files in root
    FAT_ReadFile(&part, "root/test.txt"); //Read test.txt

    //Initialize Graphics
    if (!VBE_Initialize()) {
        printf("VBE initialization error\r\n");
        goto end;
    }

    if (x86_WaitForEnter()) {
        //VBE_DrawOS(); //Demo Desktop
        VBE_SetMode(0x03); //Exit Video Mode
        kernelEntry(&g_BootParams); //Execute Kernel
    }

end:
    for (;;);
}
