#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>

static uint16_t standard_checksum(uint8_t *data, int len) {
    uint32_t sum = 0;
    uint16_t *ptr = (uint16_t *)data;
    while (len > 1) { sum += *ptr++; len -= 2; }
    if (len > 0) sum += *(uint8_t *)ptr;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return ~sum;
}

int main() {
    uint8_t full[36] = {
        10,0,2,15, 172,217,172,142, 0, 6, 0, 24,
        0xc0, 0x01, 0x00, 0x50,
        0x00, 0x00, 0x03, 0xe8,
        0x00, 0x00, 0x00, 0x00,
        0x60, 0x02, 0x20, 0x00,
        0x4e, 0x76, 0x00, 0x00,
        0x02, 0x04, 0x05, 0xb4
    };
    
    uint16_t verify = standard_checksum(full, 36);
    printf("verify = %04x\n", verify);
    return 0;
}