; kernel/arch/x86_64/isr.asm
; ISR stubs for all 256 interrupt vectors.
;
; CPU exceptions that push an error code: 8, 10-14, 17, 21, 29, 30
; All others get a dummy 0 error code pushed by us.
;
; Stack layout when isr_common is entered (low→high):
;   [rsp+ 0] int_no      (we push)
;   [rsp+ 8] err_code    (CPU or we push)
;   [rsp+16] rip         (CPU)
;   [rsp+24] cs          (CPU)
;   [rsp+32] rflags      (CPU)
;   [rsp+40] rsp_old     (CPU, if privilege change)
;   [rsp+48] ss          (CPU, if privilege change)
;
; After isr_common pushes all GPRs, the top of stack matches
; the registers_t layout defined in idt.h.

BITS 64

; ---- Macros -----------------------------------------------------------
%macro ISR_NOERR 1
global isr%1
isr%1:
    push 0          ; dummy error code
    push %1         ; interrupt number
    jmp  isr_common
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    push %1         ; interrupt number (error code already on stack)
    jmp  isr_common
%endmacro

%macro IRQ 1
global irq%1
irq%1:
    push 0
    push (%1 + 32)
    jmp  isr_common
%endmacro

; ---- CPU Exceptions (vectors 0-31) ------------------------------------
ISR_NOERR  0    ; Division By Zero
ISR_NOERR  1    ; Debug
ISR_NOERR  2    ; Non-Maskable Interrupt
ISR_NOERR  3    ; Breakpoint
ISR_NOERR  4    ; Overflow
ISR_NOERR  5    ; Bound Range Exceeded
ISR_NOERR  6    ; Invalid Opcode
ISR_NOERR  7    ; Device Not Available
ISR_ERR    8    ; Double Fault              (error code = 0)
ISR_NOERR  9    ; Coprocessor Segment Overrun (legacy, no error code)
ISR_ERR   10    ; Invalid TSS
ISR_ERR   11    ; Segment Not Present
ISR_ERR   12    ; Stack-Segment Fault
ISR_ERR   13    ; General Protection Fault
ISR_ERR   14    ; Page Fault
ISR_NOERR 15    ; Reserved
ISR_NOERR 16    ; x87 FPU Error
ISR_ERR   17    ; Alignment Check
ISR_NOERR 18    ; Machine Check
ISR_NOERR 19    ; SIMD FP Exception
ISR_NOERR 20    ; Virtualization Exception
ISR_ERR   21    ; Control Protection Exception
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERR   29    ; VMM Communication Exception
ISR_ERR   30    ; Security Exception
ISR_NOERR 31

; ---- Hardware IRQs (vectors 32-47, PIC1 + PIC2) ----------------------
IRQ  0     ; Timer (PIT)
IRQ  1     ; PS/2 Keyboard
IRQ  2     ; Cascade (PIC2)
IRQ  3     ; COM2
IRQ  4     ; COM1
IRQ  5     ; LPT2
IRQ  6     ; Floppy
IRQ  7     ; LPT1
IRQ  8     ; RTC
IRQ  9     ; ACPI
IRQ 10     ; Available
IRQ 11     ; Available
IRQ 12     ; PS/2 Mouse
IRQ 13     ; FPU (co-processor)
IRQ 14     ; ATA Primary
IRQ 15     ; ATA Secondary

; ---- Common handler ---------------------------------------------------
extern isr_handler

isr_common:
    ; Save all GPRs
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Set kernel data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax

    ; Call C handler — rdi = pointer to registers_t on stack
    mov rdi, rsp
    call isr_handler

    ; Restore GPRs
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Remove int_no and err_code
    add rsp, 16

    ; Return from interrupt
    iretq
