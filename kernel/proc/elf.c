/*
 * kernel/proc/elf.c
 * ELF64 binary loader.
 *
 * elf_load_into() maps and copies segments while the KERNEL CR3 is active.
 * Physical frames are allocated via pmm_alloc_frame() and are identity-mapped
 * (phys == virt) so we can write ELF data directly to the physical address
 * without switching to the user address space.
 */
#include "elf.h"
#include "../mm/pmm.h"
#include "../mm/vmm.h"
#include "../include/kprintf.h"

/*
 * elf_load_into — load an ELF64 binary into the given address space.
 *
 * @buffer     : pointer to the ELF binary in kernel (identity-mapped) memory
 * @pml4_phys  : physical address of the target PML4 (user process address space)
 *
 * The function runs with the KERNEL CR3 still loaded. Each PT_LOAD segment is:
 *   1. Mapped into pml4_phys with vmm_map_user()
 *   2. Copied via the identity-mapped physical address (phys == virt in kernel)
 *
 * Returns the ELF entry point virtual address, or 0 on error.
 */
uint64_t elf_load_into(const uint8_t *buffer, uint64_t pml4_phys) {
    if (!buffer) return 0;

    const elf64_ehdr_t *ehdr = (const elf64_ehdr_t *)buffer;

    /* Verify ELF magic */
    if (ehdr->magic != ELF_MAGIC) {
        kprintf("[ELF] Invalid magic number (got 0x%x, expected 0x%x)\n",
                ehdr->magic, ELF_MAGIC);
        return 0;
    }

    /* Verify class (64-bit) and machine (x86-64) */
    if (ehdr->class != 2 || ehdr->machine != 0x3E) {
        kprintf("[ELF] Not a 64-bit x86-64 executable (class=%d machine=0x%x)\n",
                ehdr->class, ehdr->machine);
        return 0;
    }

    /* Iterate over program headers */
    const elf64_phdr_t *phdr = (const elf64_phdr_t *)(buffer + ehdr->phoff);

    for (uint16_t i = 0; i < ehdr->phnum; i++) {
        if (phdr[i].type != PT_LOAD) continue;

        uint64_t vaddr  = phdr[i].vaddr;
        uint64_t memsz  = phdr[i].memsz;
        uint64_t filesz = phdr[i].filesz;
        uint64_t offset = phdr[i].offset;

        /* Page-align the segment */
        uint64_t start_page = ALIGN_DOWN(vaddr, PAGE_SIZE);
        uint64_t end_page   = ALIGN_UP(vaddr + memsz, PAGE_SIZE);
        uint64_t num_pages  = (end_page - start_page) / PAGE_SIZE;

        kprintf("[ELF]  PT_LOAD vaddr=0x%llx memsz=0x%llx filesz=0x%llx pages=%llu\n",
                (unsigned long long)vaddr, (unsigned long long)memsz,
                (unsigned long long)filesz, (unsigned long long)num_pages);

        for (uint64_t p = 0; p < num_pages; p++) {
            uint64_t page_vaddr = start_page + p * PAGE_SIZE;

            /* Allocate a physical frame */
            uint64_t phys = pmm_alloc_frame();
            if (!phys) {
                kprintf("[ELF] Out of physical memory at page %llu\n",
                        (unsigned long long)p);
                return 0;
            }

            /* Zero the frame via identity map (phys == virt in kernel) */
            kmemset((void *)(uintptr_t)phys, 0, PAGE_SIZE);

            /*
             * Copy the ELF file data that falls within this page.
             * We compare against the original (unaligned) vaddr so that
             * the correct byte offset into the segment is used.
             */
            uint64_t page_end = page_vaddr + PAGE_SIZE;

            /* Byte range in the segment that overlaps this page */
            if (page_vaddr < vaddr + filesz && page_end > vaddr) {
                /* Offset into the physical frame where copying starts */
                uint64_t frame_off = (vaddr > page_vaddr) ? (vaddr - page_vaddr) : 0;
                /* Offset into the file data */
                uint64_t file_off  = (page_vaddr > vaddr) ? (page_vaddr - vaddr) : 0;
                /* Number of bytes to copy */
                uint64_t copy_len  = PAGE_SIZE - frame_off;
                if (file_off + copy_len > filesz) {
                    copy_len = (filesz > file_off) ? (filesz - file_off) : 0;
                }

                if (copy_len > 0) {
                    kmemcpy((void *)(uintptr_t)(phys + frame_off),
                            buffer + offset + file_off,
                            copy_len);
                }
            }

            /* Map the frame into the user address space with USER+WRITABLE */
            vmm_map_user(pml4_phys, page_vaddr, phys, VMM_WRITABLE);
        }
    }

    return ehdr->entry;
}
