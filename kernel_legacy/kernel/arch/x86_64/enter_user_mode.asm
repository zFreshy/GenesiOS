; kernel/arch/x86_64/enter_user_mode.asm
; Drops into user mode via IRETQ
[BITS 64]
section .text

global enter_user_mode

; void enter_user_mode(uint64_t rip [rdi], uint64_t rsp [rsi])
enter_user_mode:
    ; Data segment for ring 3 (0x18 | 3 = 0x1B)
    mov ax, 0x1B
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Set up the stack for IRETQ
    ; Push SS (user data segment)
    push qword 0x1B
    
    ; Push RSP (user stack pointer)
    push rsi
    
    ; Push RFLAGS (enable interrupts: bit 9)
    push qword 0x202
    
    ; Push CS (user code segment: 0x20 | 3 = 0x23)
    push qword 0x23
    
    ; Push RIP (user entry point)
    push rdi
    
    ; Clear general purpose registers (optional but good for security)
    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    xor r8, r8
    xor r9, r9
    xor r10, r10
    xor r11, r11
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15
    xor rbp, rbp

    ; Return to user mode!
    swapgs
    iretq
