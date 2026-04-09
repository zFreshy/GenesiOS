/*
 * kernel/proc/process.c
 * High-level process creation (combining scheduler + VMM + ELF).
 *
 * Key invariant: ELF segments are loaded while the KERNEL CR3 is still
 * active.  Only user_mode_trampoline() switches to the process CR3 right
 * before the IRETQ — this ensures kmemcpy in elf_load_into() can always
 * see both the ELF binary (kernel RAM) and the destination frames (identity
 * mapped physical frames).
 */
#include "scheduler.h"
#include "elf.h"
#include "../mm/vmm.h"
#include "../mm/pmm.h"
#include "../include/kprintf.h"
#include "../arch/x86_64/gdt.h"

/* Defined in syscall_entry.asm — kernel stack pointer for syscall entry */
extern uint64_t g_kernel_rsp;

/* Assembly routine: drops to ring-3 via IRETQ */
extern void enter_user_mode(uint64_t rip, uint64_t rsp);

/* ------------------------------------------------------------------
 * user_mode_trampoline — scheduled as a kernel task, executes at ring-0,
 * then drops to ring-3 for the real user entry.
 * ------------------------------------------------------------------ */
static void user_mode_trampoline(void) {
    task_t *t = sched_current();
    kprintf("[PROC] user_mode_trampoline for PID %llu starting...\n", (unsigned long long)t->pid);

    /*
     * Set up the kernel stack pointer that syscall_entry.asm will use when
     * this process makes a syscall.  Must be done BEFORE iretq so that any
     * syscall (or hardware interrupt triggering an RSP0 load from TSS) works.
     */
    uint64_t kstack_top = (uint64_t)t->kernel_stack + t->kernel_stack_size;
    g_kernel_rsp = kstack_top;
    gdt_set_tss_rsp0(kstack_top);

    /* Switch to the process's own page table */
    vmm_load_cr3(t->cr3);

    kprintf("[PROC] Jumping to user mode at RIP=0x%llx RSP=0x%llx\n",
            (unsigned long long)t->user_entry,
            (unsigned long long)t->user_rsp);

    /* Jump to ring 3 */
    enter_user_mode(t->user_entry, t->user_rsp);

    /* Should never return */
}

/* ------------------------------------------------------------------
 * process_create_user — build a complete user-mode process from an ELF.
 * ------------------------------------------------------------------ */
void process_create_user(const char *name, const uint8_t *elf_data) {
    /* 1. Create a new address space — kernel CR3 remains active */
    uint64_t pml4_phys = vmm_create_address_space();
    if (!pml4_phys) {
        kprintf("[PROC] Failed to create address space\n");
        return;
    }

    /*
     * 2. Load ELF segments into pml4_phys while the KERNEL CR3 is still
     *    loaded.  elf_load_into() maps pages via vmm_map_user() and copies
     *    data using the identity-mapped physical addresses — no CR3 switch
     *    needed.
     */
    uint64_t entry = elf_load_into(elf_data, pml4_phys);
    if (!entry) {
        kprintf("[PROC] Failed to load ELF binary\n");
        return;
    }

    /*
     * 3. Allocate and map a user stack above the ELF (outside the 0-16MB
     *    identity map to keep things clean; we place it at a canonical
     *    high user-space address).
     *
     *    user_stack_top is the initial RSP value (stack grows down).
     *    We map 4 pages (16 KB) below that address.
     */
    uint64_t user_stack_top    = 0x00007FFFFFFFF000ULL;
    uint64_t user_stack_pages  = 16; /* Aumentado para 64 KB de stack */
    uint64_t user_stack_bottom = user_stack_top - (user_stack_pages * PAGE_SIZE);

    for (uint64_t i = 0; i < user_stack_pages; i++) {
        uint64_t phys = pmm_alloc_frame();
        if (!phys) {
            kprintf("[PROC] Out of memory for user stack\n");
            return;
        }
        kmemset((void *)(uintptr_t)phys, 0, PAGE_SIZE);

        uint64_t vaddr = user_stack_bottom + i * PAGE_SIZE;
        vmm_map_user(pml4_phys, vaddr, phys, VMM_WRITABLE);
    }

    /* 4. Create the scheduler task — it will run user_mode_trampoline */
    task_t *t = sched_create_task(name, user_mode_trampoline);
    if (!t) {
        kprintf("[PROC] Failed to allocate task_t\n");
        return;
    }

    /* Store the user-space entry and stack in the task so the trampoline
     * can pick them up from sched_current() */
    t->cr3        = pml4_phys;
    t->user_entry = entry;
    t->user_rsp   = user_stack_top;

    /* 5. Add to scheduler */
    sched_add(t);

    kprintf("[PROC] Created user process '%s' (PID %llu) entry=0x%llx rsp=0x%llx\n",
            name,
            (unsigned long long)t->pid,
            (unsigned long long)t->user_entry,
            (unsigned long long)t->user_rsp);
}
