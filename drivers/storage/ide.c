#include <hw/ata.h>
#include <asm/io.h>

static void ata_select_device(uint8_t drive, uint32_t lba) {
    outb(ATA_PRIMARY_IO_BASE + 6, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO_BASE + 2, 1);
    outb(ATA_PRIMARY_IO_BASE + 3, (uint8_t) (lba & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + 4, (uint8_t) ((lba >> 8) & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + 5, (uint8_t) ((lba >> 16) & 0xFF));
}

int ata_read_sector(uint8_t drive, uint32_t lba, uint8_t *buffer) {
    while (inb(ATA_PRIMARY_IO_BASE + 7) & 0x80);
    ata_select_device(drive, lba);
    outb(ATA_PRIMARY_IO_BASE + 7, ATA_CMD_READ);
    while (inb(ATA_PRIMARY_IO_BASE + 7) & 0x80);

    for (int i = 0; i < ATA_SECTOR_SIZE / 2; i++) {
        uint16_t data = inw(ATA_PRIMARY_IO_BASE);
        buffer[i * 2] = (uint8_t) (data & 0xFF);
        buffer[i * 2 + 1] = (uint8_t) ((data >> 8) & 0xFF);
    }

    return 0;
}

int ata_write_sector(uint8_t drive, uint32_t lba, const uint8_t *buffer) {
    while (inb(ATA_PRIMARY_IO_BASE + 7) & 0x80);
    ata_select_device(drive, lba);
    outb(ATA_PRIMARY_IO_BASE + 7, ATA_CMD_WRITE);
    while (inb(ATA_PRIMARY_IO_BASE + 7) & 0x80);

    for (int i = 0; i < ATA_SECTOR_SIZE / 2; i++) {
        uint16_t data = ((uint16_t) buffer[i * 2 + 1] << 8) | buffer[i * 2];
        outw(ATA_PRIMARY_IO_BASE, data);
    }

    // flush cache
    outb(ATA_PRIMARY_IO_BASE + 7, 0xE7);

    return 0;
}
