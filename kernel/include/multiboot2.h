/*
 * kernel/include/multiboot2.h
 * Minimal Multiboot2 structure definitions needed by the kernel.
 * Reference: https://www.gnu.org/software/grub/manual/multiboot2/
 */
#ifndef MULTIBOOT2_H
#define MULTIBOOT2_H

#include "kernel.h"

/* Magic placed in EAX by the bootloader */
#define MB2_BOOTLOADER_MAGIC  0x36D76289U

/* ------------------------------------------------------------------ */
/* Top-level info structure                                            */
/* ------------------------------------------------------------------ */
typedef struct {
    uint32_t total_size;
    uint32_t reserved;
    /* Followed by a sequence of tags, 8-byte aligned each */
} PACKED mb2_info_t;

/* ------------------------------------------------------------------ */
/* Generic tag header                                                  */
/* ------------------------------------------------------------------ */
typedef struct {
    uint32_t type;
    uint32_t size;
} PACKED mb2_tag_t;

/* Tag types */
#define MB2_TAG_END       0
#define MB2_TAG_CMDLINE   1
#define MB2_TAG_BOOTNAME  2
#define MB2_TAG_MMAP      6
#define MB2_TAG_FRAMEBUF  8
#define MB2_TAG_EFI64     12

/* ------------------------------------------------------------------ */
/* Memory map tag (type 6)                                             */
/* ------------------------------------------------------------------ */
typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    /* Followed by mb2_mmap_entry_t[] */
} PACKED mb2_mmap_tag_t;

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;      /* 1 = available RAM */
    uint32_t reserved;
} PACKED mb2_mmap_entry_t;

#define MB2_MMAP_AVAILABLE  1
#define MB2_MMAP_RESERVED   2
#define MB2_MMAP_ACPI_RECLM 3
#define MB2_MMAP_ACPI_NVS   4
#define MB2_MMAP_BAD        5

/* ------------------------------------------------------------------ */
/* Command line tag (type 1)                                           */
/* ------------------------------------------------------------------ */
typedef struct {
    uint32_t type;
    uint32_t size;
    char     string[0];     /* null-terminated string follows */
} PACKED mb2_cmdline_tag_t;

/* ------------------------------------------------------------------ */
/* Helper: iterate over tags                                           */
/* ------------------------------------------------------------------ */
static inline mb2_tag_t *mb2_next_tag(mb2_tag_t *tag) {
    uintptr_t next = (uintptr_t)tag + ALIGN_UP(tag->size, 8);
    return (mb2_tag_t *)next;
}

/* Find first tag of given type; returns NULL if not found */
static inline mb2_tag_t *mb2_find_tag(uint64_t mboot_info, uint32_t type) {
    mb2_info_t *info = (mb2_info_t *)(uintptr_t)mboot_info;
    mb2_tag_t  *tag  = (mb2_tag_t *)((uintptr_t)info + sizeof(mb2_info_t));

    while (tag->type != MB2_TAG_END) {
        if (tag->type == type) return tag;
        tag = mb2_next_tag(tag);
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Framebuffer info tag (type 8) — filled by GRUB when using gfx mode */
/* ------------------------------------------------------------------ */
typedef struct {
    uint32_t type;               /* = 8 */
    uint32_t size;
    uint64_t framebuffer_addr;   /* physical address of the framebuffer */
    uint32_t framebuffer_pitch;  /* bytes per scanline */
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;    /* bits per pixel  */
    uint8_t  framebuffer_type;   /* 1 = direct RGB  */
    uint16_t reserved;
    /* Color info follows for indexed modes; we only use type=1 (RGB) */
} PACKED mb2_framebuffer_tag_t;

/* Framebuffer type values */
#define MB2_FB_TYPE_INDEXED  0
#define MB2_FB_TYPE_RGB      1
#define MB2_FB_TYPE_TEXT     2

#endif /* MULTIBOOT2_H */

