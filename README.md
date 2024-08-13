# osdev
This is a 32 bit hobby operating system written from scratch. Development started at approximately August 2022, halted 3 months later and continued at June 2024.

<br>

## Features
+ GDT/IDT ✅
+ TSS ✅
+ PIT timer ✅
+ Serial debugging ✅
+ Paging ✅
+ Kernel heap memory allocator ✅
+ ACPI implementation ✅
+ VESA mode (using LFB provided by grub) ✅
+ Custom font support (using [ssfn.h](https://gitlab.com/bztsrc/scalable-font2/-/blob/master/ssfn.h?ref_type=heads)) ✅
+ PIO ATA driver for sector r/w (temporary) ✅
+ SATA driver via DMA ❌
+ Custom filesystem (currently wip) ❌
+ Network stack ❌
+ Keyboard and mouse drivers ❌
+ Usermode and system calls ❌
+ Multitasking and ELF loader ❌
+ GUI and userspace utility programs ❌

<br>

## Third party software
While this operating system uses its own native tools, it also uses [Scalable Screen Font 2.0](https://gitlab.com/bztsrc/scalable-font2/-/tree/master?ref_type=heads) for font rendering.
