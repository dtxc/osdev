# osdev
"It works on my machine"

<br>

## Features
+ GDT/IDT ✅
+ TSS ✅
+ PIT timer ✅
+ Serial debugging ✅
+ Paging ✅
+ Kernel heap memory allocator ✅
+ ACPI table parsing ✅
+ VESA mode (using LFB provided by grub) ✅
+ Custom font support (using [ssfn.h](https://gitlab.com/bztsrc/scalable-font2/-/blob/master/ssfn.h?ref_type=heads)) ✅
+ ATA driver via DMA (currently wip) ❌
+ Custom filesystem (currently wip) ❌
+ HPET ❌
+ Keyboard and mouse drivers ❌
+ Usermode and system calls ❌
+ Multitasking and ELF loader ❌
+ Network stack ❌
+ GUI and userspace utility programs ❌

<br>

## Notes
+ The LFB is originally placed at `0xFD000000` and is mapped at VA = `0xD0000000`, mapping it with PA = VA causes issues which I may resolve in the future.
+ The PIT timer is very inaccurate, an IRQ is fired every 0.999847 ms, resulting in a 153 ns error every second. (i.e. calling `pit_sleep(10000)` will finish 1530 ns earlier)
+ The ATA driver is currently implemented using programmed I/O which is significantly slower than direct memory access (DMA).

<br>

## Third party software
While this operating system uses its own native tools, it also uses [Scalable Screen Font 2.0](https://gitlab.com/bztsrc/scalable-font2/-/tree/master?ref_type=heads) for font rendering.
