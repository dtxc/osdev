#include <string.h>

#include <asm/io.h>
#include <hal/acpi.h>
#include <int/timer.h>
#include <mm/paging.h>

struct FADT *fadt;

struct RSDP *find_rsdp() {
    uint8_t *ebda = (uint8_t *) 0x40E; // extended BIOS data area

    // the signature is within a 16 byte boundary
    for (int i = 0; i < 0x10; i++) {
        if (strncmp(ebda, "RSD PTR ", 8)) {
            return (struct RSDP*) ebda;
        }
        ebda++;
    }

    // RSDP is not present in the EBDA so we search in the main BIOS area below 1 mb
    for (int i = BIOS_BASE_ADDR; i < BIOS_MAX_ADDR; i++) {
        uint8_t *ptr = (uint8_t *) i;
        if (strncmp(ptr, "RSD PTR ", 8)) {
            return (struct RSDP *) i;
        }
    }

    return NULL;
}

struct FADT *find_fadt(struct RSDT *rsdt) {
    int entries = (rsdt->header.len - sizeof(rsdt->header)) / 4;

    for (int i = 0; i < entries; i++) {
        struct ACPISTDHeader *h = (struct ACPISTDHeader *) rsdt->pointerToOtherSDT[i];
        if (!strncmp(h->signature, "FACP", 4)) {
            return (void *) h;
        }
    }

    return NULL;
}

void init_acpi() {
    struct RSDP *rsdp = find_rsdp();

    if (rsdp == NULL) {
        serial_printf("RSDP initialization failed.\n");
        return;
    }

    serial_printf("RSDP found at 0x%x\n", (uint32_t) rsdp);
    serial_printf("RSDT address: 0x%x\n", rsdp->RSDTAddress);

    // the RSDT is never larger than a page 
    map_memory(
        rsdp->RSDTAddress,
        rsdp->RSDTAddress,
        0x1000,
        kernel_dir
    );

    struct RSDT *rsdt = (struct RSDT *) rsdp->RSDTAddress;

    /*
        the other sdt pointers are before the rsdt and still no larger than a page,
        so we will unmap the current page and replace it with the new one
    */
    uint32_t first_entry = rsdt->pointerToOtherSDT[0];

    unmap_memory(rsdp->RSDTAddress, 0x1000, kernel_dir);
    map_memory(first_entry, first_entry, 0x1000, kernel_dir);

    int entries = (rsdt->header.len - sizeof(rsdt->header)) / 4;
    serial_printf("RSDT entries found: %d\n", entries);

    fadt = find_fadt(rsdt);
    serial_printf("FADT found at: 0x%x\n", (uint32_t) fadt);

    outb(fadt->SMI_commandPort, fadt->acpiEnable);
    pit_sleep(3000); // wait 3s for the hardware to change modes.

    // poll pm1a control block until bit 0 is set. when completed, power management is enabled.
    while ((inw(fadt->PM1aControlBlock) & 1) == 0);
}
