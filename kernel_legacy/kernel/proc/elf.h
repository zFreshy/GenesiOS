/*
 * kernel/proc/elf.h
 * ELF64 definitions and loader.
 */
#ifndef ELF_H
#define ELF_H

#include "../include/kernel.h"

#define ELF_MAGIC 0x464C457F /* "\x7fELF" */

typedef struct {
    uint32_t magic;
    uint8_t  class;      /* 1 = 32-bit, 2 = 64-bit */
    uint8_t  data;       /* 1 = little-endian, 2 = big-endian */
    uint8_t  version;
    uint8_t  os_abi;
    uint8_t  abi_version;
    uint8_t  pad[7];
    uint16_t type;       /* 2 = executable */
    uint16_t machine;    /* 0x3E = x86-64 */
    uint32_t e_version;
    uint64_t entry;
    uint64_t phoff;
    uint64_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} PACKED elf64_ehdr_t;

typedef struct {
    uint32_t type;       /* 1 = PT_LOAD */
    uint32_t flags;      /* 1 = X, 2 = W, 4 = R */
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
} PACKED elf64_phdr_t;

#define PT_LOAD 1

/* Load an ELF64 binary into the given address space (identified by pml4_phys).
 * Must be called with the KERNEL CR3 active.
 * Maps segments via vmm_map_user() and copies data via identity-mapped phys addresses.
 * Returns the entry point virtual address, or 0 on error. */
uint64_t elf_load_into(const uint8_t *buffer, uint64_t pml4_phys);

#endif /* ELF_H */