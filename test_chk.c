#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>

struct tcp_pseudo_header {
    uint8_t src_ip[4];
    uint8_t dst_ip[4];
    uint8_t reserved;
    uint8_t protocol;
    uint16_t tcp_length;
} __attribute__((packed));

static uint16_t calculate_tcp_checksum(struct tcp_pseudo_header *phdr, uint8_t *tcp_pkt, uint16_t tcp_len) {
    uint32_t sum = 0;
    uint16_t *ptr = (uint16_t *)phdr;
    for (int i = 0; i < sizeof(struct tcp_pseudo_header) / 2; i++) sum += ptr[i];
    ptr = (uint16_t *)tcp_pkt;
    for (int i = 0; i < tcp_len / 2; i++) sum += ptr[i];
    if (tcp_len % 2) sum += (uint16_t)tcp_pkt[tcp_len - 1];
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return ~sum;
}

int main() {
    struct tcp_pseudo_header phdr = {
        .src_ip = {10,0,2,15},
        .dst_ip = {172,217,172,142},
        .reserved = 0,
        .protocol = 6,
        .tcp_length = htons(24)
    };
    uint8_t pkt[24] = {
        0xc0, 0x01, 0x00, 0x50, // 49153, 80
        0x00, 0x00, 0x03, 0xe8, // 1000
        0x00, 0x00, 0x00, 0x00, // 0
        0x60, 0x02, 0x20, 0x00, // 6<<4, SYN, 8192
        0x00, 0x00, 0x00, 0x00, // csum, urg
        0x02, 0x04, 0x05, 0xb4  // MSS 1460
    };
    
    uint16_t csum = calculate_tcp_checksum(&phdr, pkt, 24);
    memcpy(pkt + 16, &csum, 2); // Put checksum in packet
    
    FILE *f = fopen("packet.bin", "wb");
    fwrite(pkt, 1, 24, f);
    fclose(f);
    
    return 0;
}