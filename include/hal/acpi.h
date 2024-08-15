#pragma once

#include <common.h>

#define BIOS_BASE_ADDR 0x000E0000
#define BIOS_MAX_ADDR  0x000FFFFF

#define ACPI_HEAD_DEF struct ACPISTDHeader header

struct GenericAddressStructure {
    uint8_t AddressSpace;
    uint8_t BitWidth;
    uint8_t BitOffset;
    uint8_t AccessSize;

    uint64_t addr;
} __attribute__((packed));

struct ACPISTDHeader {
    char signature[4];

    uint32_t len;
    uint8_t revision;
    uint8_t checksum;

    char OEMID[6];
    char OEMTableId[8];

    uint32_t OEMRevision;
    uint32_t creatorId;
    uint32_t creatorRev;
} __attribute__((packed));

struct FADT {
    ACPI_HEAD_DEF;
    uint32_t firmwareCtrl;
    uint32_t DSDT;

    // field used in ACPI 1.0
    uint8_t reserved;

    uint8_t preferredPowerManagmentProfile;
    uint16_t SCI_interrupt;
    uint32_t SMI_commandPort;
    uint8_t acpiEnable;
    uint8_t acpiDisable;
    uint8_t S4BIOS_REQ;
    uint8_t PSTATE_CONTROL;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t PM1EventLength;
    uint8_t PM1ControlLength;
    uint8_t PM2ControlLength;
    uint8_t PMTimerLength;
    uint8_t GPE0Length;
    uint8_t GPE1Length;
    uint8_t GPE1Base;
    uint8_t CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t DutyOffset;
    uint8_t DutyWidth;
    uint8_t DayAlarm;
    uint8_t MonthAlarm;
    uint8_t Century;

    // reserved in ACPI 1.0, in use since ACPI 2.0
    uint16_t BootArchitectureFlags;

    uint8_t reserved2;
    uint32_t flags;

    struct GenericAddressStructure ResetReg;

    uint8_t ResetValue;
    uint8_t reserved3[3];

    // 64 bit pointers, available in ACPI 2.0+
    uint64_t X_FirmwareControl;
    uint64_t X_DSDT;

    struct GenericAddressStructure X_PM1aEventBlock;
    struct GenericAddressStructure X_PM1bEventBlock;
    struct GenericAddressStructure X_PM1aControlBlock;
    struct GenericAddressStructure X_PM1bControlBlock;
    struct GenericAddressStructure X_PM2ControlBlock;
    struct GenericAddressStructure X_PMTimerBlock;
    struct GenericAddressStructure X_GPE0Block;
    struct GenericAddressStructure X_GPE1Block;
} __attribute__((packed));

struct RSDP {
   char signature[8];
   uint8_t checksum;
   char OEMID[6];
   uint8_t revision;
   uint32_t RSDTAddress;
} __attribute__((packed));

struct RSDT {
    ACPI_HEAD_DEF;
    uint32_t pointerToOtherSDT[]; // size: (h.length - sizeof(h)) / 4
} __attribute__((packed));

void init_acpi();
