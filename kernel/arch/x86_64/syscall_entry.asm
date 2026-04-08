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
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    
    ; RDI = syscall number (or RAX depending on ABI, let's say RAX = syscall number)
    ; RSI, RDX, R10, R8, R9 = args
    ; Wait, standard SysV ABI for syscalls:
    ; RAX = syscall number
    ; RDI, RSI, RDX, R10, R8, R9 = args
    ; R11 = RFLAGS, RCX = RIP
    
    ; Call C handler
    ; We can pass args directly since they are mostly in the right registers.
    ; C signature: uint64_t syscall_handler(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5)
    ; But RCX is clobbered. R10 is used for a4.
    ; So we need to move R10 to RCX for the C call.
    mov rcx, r10
    
    ; The syscall number is in RAX. We want it as first argument (RDI).
    ; But wait, RDI has a1.
    ; So let's push all args and pass a pointer to a struct, or just move things around.
    ; Let's pass a pointer to saved registers.
    push r9
    push r8
    push r10
    push rdx
    push rsi
    push rdi
    push rax
    
    mov rdi, rsp ; rdi = pointer to struct
    
    call syscall_handler
    
    ; RAX has return value.
    add rsp, 56 ; pop args
    
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
    sysretq
