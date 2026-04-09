#include "ethernet.h"
#include "arp.h"
#include "../include/kprintf.h"
#include "../include/net.h"

static inline uint16_t htons(uint16_t hostshort) {
    return (hostshort >> 8) | (hostshort << 8);
}
static inline uint16_t ntohs(uint16_t netshort) {
    return htons(netshort);
}

void ethernet_receive(uint8_t *packet, uint16_t len) {
    if (len < sizeof(struct eth_header)) return;
    
    struct eth_header *hdr = (struct eth_header *)packet;
    uint16_t type = ntohs(hdr->ethertype);
    
    if (type == ETHERTYPE_ARP) {
        arp_receive(packet + sizeof(struct eth_header), len - sizeof(struct eth_header));
    } else if (type == ETHERTYPE_IPV4) {
        kprintf("  [Net] IPv4 packet received.\n");
        // TODO: ipv4_receive(...)
    } else {
        // Unknown or unsupported protocol (e.g. IPv6, etc.)
    }
}

void ethernet_send(uint8_t *dst_mac, uint16_t ethertype, uint8_t *payload, uint16_t payload_len) {
    uint16_t total_len = sizeof(struct eth_header) + payload_len;
    if (total_len < 60) total_len = 60; /* Padding for minimum Ethernet frame size */
    
    /* TODO: Allocate dynamically or use a proper packet buffer system */
    uint8_t buffer[2048]; 
    kmemset(buffer, 0, total_len);
    
    struct eth_header *hdr = (struct eth_header *)buffer;
    for (int i = 0; i < 6; i++) {
        hdr->dst_mac[i] = dst_mac[i];
        hdr->src_mac[i] = g_net_dev.mac[i];
    }
    hdr->ethertype = htons(ethertype);
    
    if (payload && payload_len > 0) {
        kmemcpy(buffer + sizeof(struct eth_header), payload, payload_len);
    }
    
    extern int e1000_send_packet(const void *data, uint16_t len);
    e1000_send_packet(buffer, total_len);
}