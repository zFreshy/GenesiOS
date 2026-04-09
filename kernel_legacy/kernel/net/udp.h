#ifndef UDP_H
#define UDP_H

#include "../include/kernel.h"
#include "ipv4.h"

struct udp_header {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed));

void udp_receive(uint8_t *packet, uint16_t len, uint8_t *src_ip);
void udp_send(uint8_t *dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t *payload, uint16_t payload_len);

#endif /* UDP_H */