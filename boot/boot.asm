; =============================================================
; boot/boot.asm
; Genesi OS — x86-64 Bootstrap
;
; GRUB2 (Multiboot2) loads us in 32-bit protected mode.
; We must:
;   1. Verify Multiboot2 magic in EAX
;   2. Check CPUID & long-mode support
;   3. Build minimal identity-mapped page tables (16 MB, 2MB pages)
;   4. Enable PAE, load CR3, enable long mode (EFER.LME), enable paging
;   5. Load 64-bit GDT, far-jump to 64-bit code
;   6. Set up segment registers and stack, then call kernel_main
; =============================================================

BITS 32

; -------------------------------------------------------
; Multiboot2 header constants
; -------------------------------------------------------
MB2_MAGIC    equ 0xE85250D6
MB2_ARCH     equ 0                          ; i386/x86
MB2_LEN      equ (mb2_end - mb2_start)
MB2_CHECKSUM equ (0x100000000 - (MB2_MAGIC + MB2_ARCH + MB2_LEN))

; Bootloader-to-OS magic that GRUB puts in EAX
MB2_LOADER_MAGIC equ 0x36D76289

; -------------------------------------------------------
; Multiboot2 header  (must be in first 32 KB of image)
; -------------------------------------------------------
section .multiboot2
align 8
mb2_start:
    dd MB2_MAGIC
    dd MB2_ARCH
    dd MB2_LEN
    dd MB2_CHECKSUM
    ; Required end tag
    dw 0    ; type  = 0
    dw 0    ; flags = 0
    dd 8    ; size  = 8
mb2_end:

; -------------------------------------------------------
; BSS: page tables + kernel stack
; -------------------------------------------------------
section .bss
align 4096

pml4:         resb 4096       ; Page Map Level 4
pdp:          resb 4096       ; Page Directory Pointer Table
pd:           resb 4096       ; Page Directory (2 MB entries)

align 16
stack_bottom: resb 16384      ; 16 KB kernel stack
stack_top:

; -------------------------------------------------------
; 32-bit entry point
; -------------------------------------------------------
section .text
global _start
extern kernel_main

_start:
    ; GRUB passes:
    ;   EAX = MB2_LOADER_MAGIC
    ;   EBX = physical address of Multiboot2 info struct

    ; Save for later (we'll pass them to kernel_main as RDI, RSI)
    mov edi, eax        ; arg1: boot magic
    mov esi, ebx        ; arg2: mboot info ptr

    cli                 ; disable interrupts

    ; Verify Multiboot2 magic
    cmp eax, MB2_LOADER_MAGIC
    jne .err_magic

    ; Feature checks
    call cpu_check_cpuid
    call cpu_check_longmode

    ; Build page tables
    call paging_setup

    ; Enable PAE
    mov eax, cr4
    or  eax, (1 << 5)
    mov cr4, eax

    ; Point CR3 at PML4
    mov eax, pml4
    mov cr3, eax

    ; Enable Long Mode in EFER MSR (0xC0000080)
    mov ecx, 0xC0000080
    rdmsr
    or  eax, (1 << 8)   ; LME bit
    wrmsr

    ; Enable paging + keep protected mode
    mov eax, cr0
    or  eax, (1 << 31) | (1 << 0)
    mov cr0, eax

    ; Load 64-bit GDT and far-jump into long mode
    lgdt [gdt64.ptr]
    jmp  gdt64.code : long_mode_entry

; -------------------------------------------------------
; Error handlers — print one letter at VGA 0xB8000
; -------------------------------------------------------
.err_magic:
    mov dword [0xB8000], 0x4F4D4F45   ; "EM" red-on-white
    hlt
    jmp $

; -------------------------------------------------------
; CPUID support check (flip bit 21 of EFLAGS)
; -------------------------------------------------------
cpu_check_cpuid:
    pushfd
    pop  eax
    mov  ecx, eax
    xor  eax, (1 << 21)
    push eax
    popfd
    pushfd
    pop  eax
    push ecx
    popfd
    xor  eax, ecx
    jz   .no_cpuid
    ret
.no_cpuid:
    mov dword [0xB8000], 0x4F434F43   ; "CC"
    hlt
    jmp $

; -------------------------------------------------------
; Long-mode availability check via extended CPUID
; -------------------------------------------------------
cpu_check_longmode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb  .no_lm
    mov eax, 0x80000001
    cpuid
    test edx, (1 << 29)    ; LM bit
    jz  .no_lm
    ret
.no_lm:
    mov dword [0xB8000], 0x4F4C4F4C   ; "LL"
    hlt
    jmp $

; -------------------------------------------------------
; Set up identity-mapped page tables (first 16 MB)
; Using 2 MB huge pages: PML4 -> PDP -> PD
; -------------------------------------------------------
paging_setup:
    ; Zero all three tables
    mov edi, pml4
    mov ecx, (4096 * 3) / 4
    xor eax, eax
    rep stosd

    ; PML4[0] -> PDP  (P + RW)
    mov eax, pdp
    or  eax, 0x03
    mov [pml4], eax

    ; PDP[0] -> PD  (P + RW)
    mov eax, pd
    or  eax, 0x03
    mov [pdp], eax

    ; PD[0..7]: identity-map 8 x 2 MB = 16 MB  (P + RW + PS)
    mov ecx, 0
    mov eax, 0x83           ; present | writable | huge
.pd_loop:
    mov [pd + ecx * 8], eax
    add eax, 0x200000       ; +2 MB
    inc ecx
    cmp ecx, 8
    jne .pd_loop
    ret

; -------------------------------------------------------
; 64-bit GDT  (minimal: null + kernel code + kernel data)
; -------------------------------------------------------
section .data
align 8
gdt64:
    dq 0                        ; 0x00  null descriptor
.code: equ $ - gdt64            ; 0x08  kernel code
    dq 0x00AF9A000000FFFF       ;   P=1, DPL=0, L=1 (64-bit), E=1, R=1
.data: equ $ - gdt64            ; 0x10  kernel data
    dq 0x00AF92000000FFFF       ;   P=1, DPL=0, W=1
gdt64_end:

.ptr:
    dw gdt64_end - gdt64 - 1   ; limit
    dd gdt64                    ; base (low 32 bits — fine before paging)
    dd 0                        ; base high (padding, ignored by 32-bit lgdt)

; -------------------------------------------------------
; 64-bit long-mode entry
; -------------------------------------------------------
BITS 64

section .text
long_mode_entry:
    ; Load data segment selectors
    mov ax, gdt64.data
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Set up kernel stack
    mov rsp, stack_top

    ; RDI = magic (from EDI, zero-extended)
    ; RSI = mboot info ptr (from ESI, zero-extended)
    ; Both already set in 32-bit code above.

    call kernel_main

    ; Should never return — halt forever
    cli
.halt:
    hlt
    jmp .halt
