/* user/test/linker.ld */
ENTRY(_start)

SECTIONS {
    . = 0x40000000; /* 1 GB — uses PDP[1], fully separate from kernel's PDP[0] */

    .text : {
        *(.text)
        *(.text.*)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    .data : {
        *(.data)
        *(.data.*)
    }

    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }

    /DISCARD/ : {
        *(.eh_frame)
        *(.note*)
    }
}
