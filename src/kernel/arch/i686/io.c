#include "io.h"

void i686_iowait() {
    i686_outb(UNUSED_PORT, 0);
}

void i686_ata_wait_bsy(uint16_t ioBase) {
    while (i686_inb(ioBase + ATA_REG_STATUS) & ATA_SR_BSY);
}

void i686_ata_wait_drq(uint16_t ioBase) {
    while (!(i686_inb(ioBase + ATA_REG_STATUS) & ATA_SR_DRQ));
}

void i686_floppy_wait_bsy() {
    while ((i686_inb(FLOPPY_STATUS) & FLOPPY_BSY));
}