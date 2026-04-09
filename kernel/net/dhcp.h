#ifndef DHCP_H
#define DHCP_H

#include "../include/kernel.h"

struct dhcp_packet {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint8_t ciaddr[4];
    uint8_t yiaddr[4];
    uint8_t siaddr[4];
    uint8_t giaddr[4];
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint32_t magic_cookie;
    uint8_t options[312];
} __attribute__((packed));

void dhcp_init(void);
void dhcp_receive(uint8_t *packet, uint16_t len);
void dhcp_send_discover(void);

#endif /* DHCP_H */