; ============================================================
; kernel/arch/x86_64/context_switch.asm
; void switch_context(uint64_t *old_rsp, uint64_t new_rsp)
;
; Saves callee-saved registers + CR3 onto the current stack,
; stores RSP into *old_rsp, loads new_rsp, restores registers,
; and returns to the new task.
; ============================================================
[BITS 64]
section .text

global switch_context

; void switch_context(uint64_t *old_rsp  [rdi],
;                     uint64_t  new_rsp  [rsi])
switch_context:
    ; Save callee-saved registers on the current stack
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    ; Save CR3 (page table base)
    mov rax, cr3
    push rax

    ; Save current RSP into *old_rsp
    mov [rdi], rsp

    ; Load new task's RSP
    mov rsp, rsi

    ; Restore CR3 — only reload if different (avoids TLB flush)
    pop rax
    mov rcx, cr3
    cmp rax, rcx
    je .skip_cr3
    mov cr3, rax
.skip_cr3:

    ; Restore callee-saved registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp

    ; ret pops the return address — for a new task this is the
    ; entry point set up by process_create().
    ret
