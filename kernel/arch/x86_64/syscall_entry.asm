[BITS 64]
section .text

global syscall_entry
extern syscall_handler

; We will use a per-CPU or global variable to store the kernel stack pointer
global g_kernel_rsp
global g_user_rsp

section .data
align 8
g_kernel_rsp: dq 0
g_user_rsp:   dq 0

section .text
syscall_entry:
    ; Save user RSP
    mov [rel g_user_rsp], rsp
    
    ; Load kernel RSP (set by scheduler on task switch)
    mov rsp, [rel g_kernel_rsp]
    
    ; Save registers used by C ABI and syscall ABI
    push rcx ; user RIP (saved by SYSCALL)
    push r11 ; user RFLAGS (saved by SYSCALL)
    
    ; Save extra registers just to be safe
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    
    ; SysV ABI for syscalls:
    ; RAX = syscall number
    ; RDI, RSI, RDX, R10, R8, R9 = args
    ; C signature: uint64_t syscall_handler(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5)
    ; In C ABI, args go to RDI, RSI, RDX, RCX, R8, R9.
    
    ; Save everything before we mess up the registers
    push rax
    push rdi
    push rsi
    push rdx
    push r10
    push r8
    push r9
    
    ; Now map Syscall ABI to C ABI:
    ; num = RAX -> RDI
    ; a1 = RDI -> RSI
    ; a2 = RSI -> RDX
    ; a3 = RDX -> RCX
    ; a4 = R10 -> R8
    ; a5 = R8  -> R9
    
    mov r9, r8
    mov r8, r10
    mov rcx, rdx
    mov rdx, rsi
    mov rsi, rdi
    mov rdi, rax
    
    call syscall_handler
    
    ; RAX has return value. Restore original registers except RAX
    pop r9
    pop r8
    pop r10
    pop rdx
    pop rsi
    pop rdi
    add rsp, 8 ; throw away pushed rax
    
    ; Restore callee-saved
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    pop r11
    pop rcx
    
    ; Restore user RSP
    mov rsp, [rel g_user_rsp]
    
    ; Return to user mode
    o64 sysret
