#ifndef ICMP_H
#define ICMP_H

#include "../include/kernel.h"
#include "ipv4.h"

struct icmp_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
} __attribute__((packed));

#define ICMP_ECHO_REPLY   0
#define ICMP_ECHO_REQUEST 8

void icmp_receive(uint8_t *packet, uint16_t len, uint8_t *src_ip);
void icmp_send_reply(uint8_t *dst_ip, uint16_t id, uint16_t seq, uint8_t *payload, uint16_t payload_len);
void icmp_send_request(uint8_t *dst_ip, uint16_t id, uint16_t seq, uint8_t *payload, uint16_t payload_len);

#endif /* ICMP_H */