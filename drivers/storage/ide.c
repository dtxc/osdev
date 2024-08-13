#include <hw/ata.h>
#include <asm/io.h>
#include <int/timer.h>

ide_channel_t channels[2];
ide_device_t devices[4];

uint8_t ide_buf[2048] = {0};
static volatile uint8_t ide_irq_invoked = 0;
static uint8_t atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void ide_write(uint8_t channel, uint8_t reg, uint8_t data) {
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   if (reg < 0x08)
      outb(channels[channel].base  + reg - 0x00, data);
   else if (reg < 0x0C)
      outb(channels[channel].base  + reg - 0x06, data);
   else if (reg < 0x0E)
      outb(channels[channel].ctrl  + reg - 0x0A, data);
   else if (reg < 0x16)
      outb(channels[channel].bmide + reg - 0x0E, data);
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

uint8_t ide_read(uint8_t channel, uint8_t reg) {
   uint8_t result;
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   if (reg < 0x08)
      result = inb(channels[channel].base + reg - 0x00);
   else if (reg < 0x0C)
      result = inb(channels[channel].base  + reg - 0x06);
   else if (reg < 0x0E)
      result = inb(channels[channel].ctrl  + reg - 0x0A);
   else if (reg < 0x16)
      result = inb(channels[channel].bmide + reg - 0x0E);
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
   return result;
}

uint8_t ide_polling(uint8_t channel, uint8_t advanced_check) {
    for (int i = 0; i < 4; i++) {
        ide_read(channel, ATA_REG_ALTSTATUS); // reading alternate status takes 100 ns
    }

    // wait for BSY to be cleared
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY);

    if (advanced_check) {
        uint8_t state = ide_read(channel, ATA_REG_STATUS);

        if (state & ATA_SR_ERR) {
            return 2;
        } else if (state & ATA_SR_DF) {
            return 1;
        } else if ((state & ATA_SR_DRQ) == 0) {
            return 3;
        }
    }

    return 0;
}

void ide_read_buffer(uint8_t channel, uint8_t reg, uint32_t *buffer, uint32_t quads) {
   /* WARNING: This code contains a serious bug. The inline assembly trashes ES and
    *           ESP for all of the code the compiler generates between the inline
    *           assembly blocks.
    */
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   asm("pushw %es; movw %ds, %ax; movw %ax, %es");
   if (reg < 0x08)
      insl(channels[channel].base  + reg - 0x00, buffer, quads);
   else if (reg < 0x0C)
      insl(channels[channel].base  + reg - 0x06, buffer, quads);
   else if (reg < 0x0E)
      insl(channels[channel].ctrl  + reg - 0x0A, buffer, quads);
   else if (reg < 0x16)
      insl(channels[channel].bmide + reg - 0x0E, buffer, quads);
   asm("popw %es;");
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

void init_ide(uint32_t BAR0, uint32_t BAR1, uint32_t BAR2, uint32_t BAR3, uint32_t BAR4) {
    // detect IDE I/O ports.
    channels[ATA_PRIMARY].base  = (BAR0 & 0xFFFFFFFC) + 0x1F0 * (!BAR0);
    channels[ATA_PRIMARY].ctrl  = (BAR1 & 0xFFFFFFFC) + 0x3F6 * (!BAR1);
    channels[ATA_PRIMARY].bmide = (BAR4 & 0xFFFFFFFC) + 0;

    channels[ATA_SECONDARY].base    = (BAR2 & 0xFFFFFFFC) + 0x170 * (!BAR2);
    channels[ATA_SECONDARY].ctrl    = (BAR3 & 0xFFFFFFFC) + 0x376 * (!BAR3);
    channels[ATA_SECONDARY].bmide   = (BAR4 & 0xFFFFFFFC) + 8;

    // disable IRQs
    ide_write(ATA_PRIMARY, ATA_REG_CONTROL, 2);
    ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

    // detect ATA/ATAPI devices
    int count = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            uint8_t err = 0;
            uint8_t type = IDE_ATA;
            uint8_t status;
            devices[count].reserved = 0;

            // select drive
            ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
            pit_sleep(1); // wait for drive selection to apply

            // request identification
            ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

            // poll until status is set, if status = 0 then no device was found.
            if (ide_read(i, ATA_REG_STATUS) == 0) continue;

            while (1) {
                status = ide_read(i, ATA_REG_STATUS);
                if (status & ATA_SR_ERR) { // device is not ATA
                    err = 1;
                    break;
                }

                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break;
            }

            // probe for ATAPI devices
            if (err != 0) {
                uint8_t cl = ide_read(i, ATA_REG_LBA1);
                uint8_t ch = ide_read(i, ATA_REG_LBA2);

                if (cl == 0x14 && ch == 0xEB) {
                    type = IDE_ATAPI;
                } else if (cl == 0x69 && ch == 0x96) {
                    type = IDE_ATAPI;
                } else {
                    continue; // could not determine device type.
                }

                ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                pit_sleep(1);
            }

            ide_read_buffer(i, ATA_REG_DATA, (uint32_t *) ide_buf, 128);

            devices[count].reserved     = 1;
            devices[count].type         = type;
            devices[count].channel      = i;
            devices[count].drive        = j;
            devices[count].signature    = *((uint16_t *) (ide_buf + ATA_IDENT_DEVICETYPE));
            devices[count].capabilities = *((uint16_t *) (ide_buf + ATA_IDENT_CAPABILITIES));
            devices[count].commandSets  = *((uint32_t *) (ide_buf + ATA_IDENT_COMMANDSETS));

            // get drive size
            if (devices[count].commandSets & (1 << 26)) {
                // if device uses 48 bit addressing
                devices[count].size = *((uint32_t *) (ide_buf + ATA_IDENT_MAX_LBA_EXT));
            } else {
                // device uses chs or 28 bit addressing
                devices[count].size = *((uint32_t *) (ide_buf + ATA_IDENT_MAX_LBA));
            }

            for (int k = 0; k < 40; k+=2) {
                devices[count].model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
                devices[count].model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];
            }
            devices[count].model[40] = 0;
            
            count++;
        }
    }

    // print summary
    for (int i = 0; i < 4; i++) {
        if (devices[i].reserved == 1) {
            serial_printf(
                "Found %s drive %d MB - %s\n",
                (const char *[]){"ATA", "ATAPI"}[devices[i].type],
                devices[i].size / 1024 / 2,
                devices[i].model
            );
        }
    }
}
