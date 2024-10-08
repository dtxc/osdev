global gdt_flush
global idt_flush
global tss_flush

bits 32

gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush
.flush:
    ret

idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

tss_flush:
    mov ax, 0x2B
    ltr ax
    ret