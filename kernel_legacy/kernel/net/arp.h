#ifndef ARP_H
#define ARP_H

#include "../include/kernel.h"

#define ARP_HW_ETHERNET 1
#define ARP_OP_REQUEST  1
#define ARP_OP_REPLY    2

struct arp_header {
    uint16_t hw_type;
    uint16_t proto_type;
    uint8_t  hw_len;
    uint8_t  proto_len;
    uint16_t opcode;
    uint8_t  src_mac[6];
    uint8_t  src_ip[4];
    uint8_t  dst_mac[6];
    uint8_t  dst_ip[4];
} __attribute__((packed));

extern uint8_t g_gateway_mac[6];

void arp_receive(uint8_t *packet, uint16_t len);
void arp_send_reply(uint8_t *dst_mac, uint8_t *dst_ip);
void arp_send_request(uint8_t *target_ip);

#endif /* ARP_H */