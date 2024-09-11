#include "disk.h"
#include "stdio.h"
#include "../../kernel/arch/i686/io.h"

bool Kernel_DISK_Initialize(DISK* disk, uint8_t driveNumber) {

    disk->id = driveNumber;
    if (disk->id < 0x80) {
        if (!Floppy_Initialize(disk)) {
            return false;
        }
    } else {
        if (!Hard_DISK_Initialize(disk)) {
            return false;
        }
    }

    return true;
}

bool Floppy_Initialize(DISK* disk) {

    disk->cylinders = 80;
    disk->heads = 2;
    disk->sectors = 18;

    printf("Floppy disk detected.\n");

    return true;
}

bool Hard_DISK_Initialize(DISK* disk) {
    uint16_t buffer[256];

    if (!Disk_Read(disk, 0, 1, &buffer, true)) {
        printf("No drive found.\n");
        return false;
    }

    if ((buffer[0] & (1 << 15)) && (buffer[0] & (1 << 7))) {
        printf("ATA drive detected.\n");
    } else {
        printf("IDE drive detected.\n");
    }

    disk->cylinders = buffer[1];
    disk->heads = buffer[3];
    disk->sectors = buffer[6];

    return true;
}

//TODO: Fix Floppy_Read
bool Floppy_Read(DISK* disk, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t sectors, void* buffer) {
    /*i686_outb(FLOPPY_DOR, 0x00);
    i686_outb(FLOPPY_DOR, 0x0C);

    i686_outb(FLOPPY_DATA, CMD_SPECIFY);
    i686_outb(FLOPPY_DATA, 0xDF);

    i686_floppy_wait_irq();

    i686_outb(FLOPPY_DATA, CMD_SENSE_DRIVE_STATUS);

    i686_floppy_wait_irq();

    i686_outb(FLOPPY_DATA, CMD_READ_DATA);
    i686_outb(FLOPPY_DATA, head << 2);
    i686_outb(FLOPPY_DATA, cylinder);
    i686_outb(FLOPPY_DATA, head);
    i686_outb(FLOPPY_DATA, sector);
    i686_outb(FLOPPY_DATA, 2);

    i686_floppy_wait_irq();

    for (int i = 0; i < 256; ++i) {
        buffer[i] = i686_inb(FLOPPY_DATA);
    }

    uint8_t status = i686_inb(FLOPPY_STATUS);
    if (status & (FLOPPY_BSY | STATUS_FAULT)) {
        return false;
    }*/

    return true;
}

bool Disk_Read(DISK* disk, uint32_t lba, uint8_t sectors, void* buffer, bool isIdentify) {
    uint16_t ioBase = (disk->id < 0x82) ? ATA_PRIMARY_IO_BASE : ATA_SECONDARY_IO_BASE;
    uint8_t device = (disk->id & 0x01) << 4;

    i686_outb(ioBase + ATA_REG_DEVICE, 0xE0 | (disk->id << 4) | ((lba >> 24) & 0x0F)); //Select drive and head

    i686_ata_wait_bsy(ioBase);

    for (int i = 0; i < 4; i++) i686_inb(ioBase + ATA_REG_ALTSTATUS);

    if (isIdentify) { //If the IDENTIFY command, set up the command and issue it
        i686_outb(ioBase + ATA_REG_SECTOR_COUNT, 0);
        i686_outb(ioBase + ATA_REG_LBA_LOW, 0);
        i686_outb(ioBase + ATA_REG_LBA_MID, 0);
        i686_outb(ioBase + ATA_REG_LBA_HIGH, 0);
        i686_outb(ioBase + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    } else { //For normal read operations, set up LBA and sector count
        i686_outb(ioBase + ATA_REG_SECTOR_COUNT, sectors);
        i686_outb(ioBase + ATA_REG_LBA_LOW, (uint8_t)lba);
        i686_outb(ioBase + ATA_REG_LBA_MID, (uint8_t)(lba >> 8));
        i686_outb(ioBase + ATA_REG_LBA_HIGH, (uint8_t)(lba >> 16));
        i686_outb(ioBase + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    }

    for (uint8_t sector = 0; sector < sectors; sector++) {
        i686_ata_wait_bsy(ioBase);
        i686_ata_wait_drq(ioBase);

        uint16_t* u16Buffer = (uint16_t*)((uint8_t*)buffer + sector * 512);
        for (int i = 0; i < 256; i++) {
            u16Buffer[i] = i686_inw(ioBase + ATA_REG_DATA);
        }
    }

    uint8_t status = i686_inb(ioBase + ATA_REG_STATUS);
    if (status & ATA_SR_ERR) {
        return false;
    }

    return true;
}

bool Kernel_DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, void* dataOut, bool isIdentify) {
    uint16_t cylinder, sector, head;
    DISK_LBA2CHS(disk, lba, &cylinder, &sector, &head);

    for (int i = 0; i < 3; i++) {
        if (disk->id < 0x80) {
            if (Floppy_Read(disk, cylinder, sector, head, sectors, dataOut)) {
                return true;
            }
        } else {
            if (Disk_Read(disk, lba, sectors, dataOut, isIdentify)) {
                return true;
            }
        }

        //x86_Disk_Reset(disk->id); //TODO: Make for error handling
    }

    return false;
}