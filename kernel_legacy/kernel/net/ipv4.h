#ifndef IPV4_H
#define IPV4_H

#include "../include/kernel.h"
#include "ethernet.h"

struct ipv4_header {
    uint8_t ihl : 4;
    uint8_t version : 4;
    uint8_t tos;
    uint16_t length;
    uint16_t id;
    uint16_t flags_frag;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint8_t src_ip[4];
    uint8_t dst_ip[4];
} __attribute__((packed));

void ipv4_receive(uint8_t *packet, uint16_t len);
void ipv4_send(uint8_t *dst_ip, uint8_t protocol, uint8_t *payload, uint16_t payload_len);

#endif /* IPV4_H */