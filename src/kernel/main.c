#include <stdint.h>
#include "hal/hal.h"
#include "arch/i686/irq.h"
#include "../libs/core/stdio.h"
#include "../libs/boot/memory.h"
#include "../libs/boot/bootparams.h"
#include "../libs/boot/mbr.h"
#include "../libs/core/disk.h"
#include "../libs/core/fat.h"
#include "../libs/core/cli.h"

extern void _init();

void start(BootParams* bootParams) {
    _init();

    //Initialize HAL
    HAL_Initialize();

    //Initialize DISK
    DISK disk;
    if (!Kernel_DISK_Initialize(&disk, bootParams->BootDevice)) {
        printf("Disk initialization error\r\n");
        goto end;
    }

    //Detect Partition
    Partition part;
    uint8_t mbrBuffer[512];
    Kernel_DISK_ReadSectors(&disk, 0, 1, mbrBuffer, false);
    MBR_Entry* entries = (MBR_Entry*)(mbrBuffer + 446);
    MBR_DetectPartition(&part, &disk, &entries[0]);

    //Initialize FAT
    if (!Kernel_FAT_Initialize(&part)) {
        printf("FAT initialization error\r\n");
        goto end;
    }

    clrscr();

    //Command Line
    Command_Line_Interface(bootParams, &part);

end:
    for (;;);
}