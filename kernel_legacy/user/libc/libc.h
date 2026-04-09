#ifndef LIBC_H
#define LIBC_H

#include <stdint.h>
#include <stddef.h>

// Syscalls
void exit(int status);
void print(const char *str);

// String operations
int strlen(const char *str);
void* memcpy(void *dest, const void *src, size_t n);
void* memset(void *s, int c, size_t n);

#endif