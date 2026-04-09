#include "libc.h"

// Basic syscall wrappers
// In Genesi OS, syscalls use the `syscall` instruction
// RAX = syscall number
// RDI = arg1, RSI = arg2, RDX = arg3

static inline uint64_t syscall1(uint64_t num, uint64_t arg1) {
    uint64_t ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline uint64_t syscall2(uint64_t num, uint64_t arg1, uint64_t arg2) {
    uint64_t ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1), "S"(arg2)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void exit(int status) {
    syscall1(0, (uint64_t)status); // Syscall 0 is exit in Genesi
    while(1);
}

void print(const char *str) {
    syscall2(1, (uint64_t)str, (uint64_t)strlen(str)); // Syscall 1 is write/print
}