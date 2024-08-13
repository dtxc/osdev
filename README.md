# osdev
"It works on my machine"

<br>

## Features & TODOs
+ GDT/IDT ✅
+ TSS ✅
+ PIT timer ✅
+ Serial debugging ✅
+ Paging ✅
+ Kernel heap memory allocator ✅
+ ACPI table parsing ✅
+ VESA mode (using LFB provided by grub) ✅
+ Custom font support (using [ssfn.h](https://gitlab.com/bztsrc/scalable-font2/-/blob/master/ssfn.h?ref_type=heads)) ✅
+ Simple ATA driver with sector I/O ✅
+ Custom block based filesystem (wip) ❌
+ /dev/stdout implementation* ❌
+ VESA text mode scrolling ❌
+ Keyboard and mouse drivers ❌
+ Usermode and system calls ❌
+ Multitasking and ELF loader ❌
+ Network stack ❌
+ GUI and userspace utility programs ❌

<sub>*implementing a /dev/stdout will eliminate the need for individual printf's for each display mode. I know this can be achieved without /dev/stdout but this implementation will be useful for later.</sub>

<br>

## Notes
+ The LFB is originally placed at `0xFD000000` and is mapped at VA = `0xD0000000`, mapping it with PA = VA causes issues which I may resolve in the future.
+ The PIT timer is inaccurate, an IRQ is fired every 0.999847 ms, resulting in a 153 ns error every second. (i.e. calling `pit_sleep(10000)` will finish 1530 ns earlier)
+ The ATA driver is currently implemented using programmed I/O which is significantly slower than direct memory access (DMA).

<br>

## Third party software
While this operating system uses its own native tools, it also uses [Scalable Screen Font 2.0](https://gitlab.com/bztsrc/scalable-font2/-/tree/master?ref_type=heads) for font rendering.
