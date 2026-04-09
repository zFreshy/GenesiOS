// user/test/main.c
#include <stdint.h>

/* Very basic syscall stubs for testing */
static inline uint64_t syscall(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
    uint64_t ret;
    register uint64_t r10 __asm__("r10") = a4;
    register uint64_t r8  __asm__("r8")  = a5;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void write(int fd, const char *str, uint64_t count) {
    syscall(1, fd, (uint64_t)str, count, 0, 0);
}

uint64_t getpid(void) {
    return syscall(3, 0, 0, 0, 0, 0);
}

void exit(int status) {
    syscall(0, status, 0, 0, 0, 0);
    while (1);
}

void _start(void) {
    const char *msg = "Hello from User Space (Ring 3) via SYSCALL!\n";
    write(1, msg, 44);

    uint64_t pid = getpid();
    if (pid > 0) {
        write(1, "Successfully got PID.\n", 22);
    }

    /* Try to do something illegal (should cause GPF) */
    // __asm__ volatile ("inb $0x60, %al");

    exit(42);
}
