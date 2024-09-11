#pragma once
#include <stdint.h>

#define UNUSED_PORT 0x80

/*
* Floppy Disk
*/

// I/O Ports for Floppy Disk Controller
#define FLOPPY_DATA     0x3F5    // Data register
#define FLOPPY_STATUS   0x3F4    // Status register
#define FLOPPY_COMMAND  0x3F7    // Command register
#define FLOPPY_DOR      0x3F2    // Digital Output Register

//Floppy Disk commands
#define CMD_SPECIFY             0x03   // Specify parameters
#define CMD_RECALIBRATE         0x07
#define CMD_READ_SECTOR         0x06   // Read sector command
#define CMD_WRITE_SECTOR        0x05   // Write sector command
#define CMD_SENSE_DRIVE_STATUS  0x04   // Sense drive status command
#define CMD_READ_DATA           0xE6   // Read data command for floppy drive
#define CMD_READ_ID             0x0A

//Status
#define FLOPPY_BSY            0x10    // Bit 4: Busy
#define STATUS_DATA_REQUEST   0x08    // Bit 3: Data Request
#define STATUS_FAULT          0x80    // Bit 7: Fault

/*
* ATA
*/

// ATA I/O ports for the primary and secondary channels
#define ATA_PRIMARY_IO_BASE   0x1F0
#define ATA_SECONDARY_IO_BASE 0x170
#define ATA_REG_ALTSTATUS     0x206
#define ATA_REG_DATA          0x00
#define ATA_REG_ERROR         0x01
#define ATA_REG_SECTOR_COUNT  0x02
#define ATA_REG_LBA_LOW       0x03
#define ATA_REG_LBA_MID       0x04
#define ATA_REG_LBA_HIGH      0x05
#define ATA_REG_DEVICE        0x06
#define ATA_REG_COMMAND       0x07
#define ATA_REG_STATUS        0x07

//status flags
#define ATA_SR_BSY   0x80
#define ATA_SR_DRDY  0x40
#define ATA_SR_DF    0x20
#define ATA_SR_DSC   0x10
#define ATA_SR_DRQ   0x08
#define ATA_SR_CORR  0x04
#define ATA_SR_IDX   0x02
#define ATA_SR_ERR   0x01

// ATA commands
#define ATA_CMD_IDENTIFY    0xEC
#define ATA_CMD_READ_PIO    0x20
#define ATA_CMD_WRITE_PIO   0x30

/*
* Interrupts
*/

#define DATA_PORT 0x60
#define STATUS_PORT 0x64

void __attribute__((cdecl)) i686_outb(uint16_t port, uint8_t value);
void __attribute__((cdecl)) i686_outw(uint16_t port, uint16_t value);
uint8_t __attribute__((cdecl)) i686_inb(uint16_t port);
uint16_t __attribute__((cdecl)) i686_inw(uint16_t port);
uint8_t __attribute__((cdecl)) i686_EnableInterrupts();
uint8_t __attribute__((cdecl)) i686_DisableInterrupts();
void __attribute__((cdecl)) i686_Panic();
void i686_iowait();
void i686_ata_wait_bsy(uint16_t ioBase);
void i686_ata_wait_drq(uint16_t ioBase);
void i686_floppy_wait_bsy();