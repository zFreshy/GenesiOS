; kernel/arch/x86_64/gdt_flush.asm
; Assembly stubs to reload GDT and TSS registers.

BITS 64

global gdt_flush
global tss_flush

; void gdt_flush(uint64_t gdt_ptr_addr)
; rdi = pointer to gdt_ptr_t (limit + base)
gdt_flush:
    lgdt [rdi]

    ; Reload CS via far-return trick
    push 0x08                   ; kernel code selector
    lea  rax, [rel .reload_cs]
    push rax
    retfq                       ; pops RIP then CS

.reload_cs:
    ; Reload data segments with kernel data selector (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

; void tss_flush(void)
; Loads the Task Register with the TSS selector (0x28)
tss_flush:
    mov ax, 0x28
    ltr ax
    ret
