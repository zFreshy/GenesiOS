#ifndef DNS_H
#define DNS_H

#include "../include/kernel.h"

struct dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} __attribute__((packed));

void dns_receive(uint8_t *packet, uint16_t len);
void dns_resolve(const char *domain);

#endif /* DNS_H */