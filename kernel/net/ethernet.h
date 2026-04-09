#ifndef ETHERNET_H
#define ETHERNET_H

#include "../include/kernel.h"

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_ARP  0x0806
#define ETHERTYPE_IPV6 0x86DD

struct eth_header {
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype; // big-endian
} __attribute__((packed));

void ethernet_receive(uint8_t *packet, uint16_t len);
void ethernet_send(uint8_t *dst_mac, uint16_t ethertype, uint8_t *payload, uint16_t payload_len);

#endif /* ETHERNET_H */