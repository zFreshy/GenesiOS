/* linker.ld - Genesi OS Kernel Linker Script */

ENTRY(_start)
OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
    /* Kernel loaded at 1MB physical / virtual */
    . = 1M;

    .boot :
    {
        /* Multiboot2 header MUST be in the first 32KB */
        KEEP(*(.multiboot2))
    }

    .text :
    {
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
    }

    .data :
    {
        *(.data*)
    }

    .bss :
    {
        *(COMMON)
        *(.bss*)
    }

    /* Symbol marking end of kernel image */
    kernel_end = .;

    /DISCARD/ :
    {
        *(.eh_frame)
        *(.note.*)
        *(.comment)
    }
}
