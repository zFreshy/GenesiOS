#include <stdio.h>
#include <stdint.h>

struct ipv4_header {
    uint8_t ihl : 4;
    uint8_t version : 4;
} __attribute__((packed));

int main() {
    struct ipv4_header ip;
    ip.version = 4;
    ip.ihl = 5;
    printf("byte: %02x\n", *(uint8_t*)&ip);
    return 0;
}