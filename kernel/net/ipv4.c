#include "ipv4.h"
#include "udp.h"
#include "../include/kprintf.h"
#include "../include/net.h"

static inline uint16_t htons(uint16_t hostshort) {
    return (hostshort >> 8) | (hostshort << 8);
}
static inline uint16_t ntohs(uint16_t netshort) {
    return htons(netshort);
}

static uint16_t calculate_checksum(void *addr, int count) {
    register uint32_t sum = 0;
    uint16_t * ptr = addr;

    while (count > 1) {
        sum += *ptr++;
        count -= 2;
    }

    if (count > 0) {
        sum += *(uint8_t *)ptr;
    }

    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return ~sum;
}

void ipv4_receive(uint8_t *packet, uint16_t len) {
    if (len < sizeof(struct ipv4_header)) return;
    
    struct ipv4_header *ipv4 = (struct ipv4_header *)packet;
    
    /* We only care about IPv4 right now */
    if (ipv4->version != 4) return;
    
    uint16_t total_len = ntohs(ipv4->length);
    if (len < total_len) return; // truncated packet
    
    uint16_t header_len = ipv4->ihl * 4;
    
    /* Discard packets not destined to us or broadcast (255.255.255.255) */
    bool match = true;
    bool broadcast = true;
    for (int i = 0; i < 4; i++) {
        if (ipv4->dst_ip[i] != g_net_dev.ip[i]) match = false;
        if (ipv4->dst_ip[i] != 255) broadcast = false;
    }
    
    if (!match && !broadcast && g_net_dev.ip[0] != 0) {
        return; /* Not for us */
    }
    
    uint8_t *payload = packet + header_len;
    uint16_t payload_len = total_len - header_len;
    
    if (ipv4->protocol == 17) { /* UDP */
        udp_receive(payload, payload_len, ipv4->src_ip);
    } else if (ipv4->protocol == 1) { /* ICMP */
        extern void icmp_receive(uint8_t *packet, uint16_t len, uint8_t *src_ip);
        icmp_receive(payload, payload_len, ipv4->src_ip);
    }
}

void ipv4_send(uint8_t *dst_ip, uint8_t protocol, uint8_t *payload, uint16_t payload_len) {
    uint16_t total_len = sizeof(struct ipv4_header) + payload_len;
    
    uint8_t buffer[2048];
    kmemset(buffer, 0, total_len);
    
    struct ipv4_header *ipv4 = (struct ipv4_header *)buffer;
    ipv4->version = 4;
    ipv4->ihl = 5;
    ipv4->tos = 0;
    ipv4->length = htons(total_len);
    ipv4->id = htons(1); /* Arbitrary ID */
    ipv4->flags_frag = 0;
    ipv4->ttl = 64;
    ipv4->protocol = protocol;
    
    for (int i = 0; i < 4; i++) {
        ipv4->src_ip[i] = g_net_dev.ip[i];
        ipv4->dst_ip[i] = dst_ip[i];
    }
    
    ipv4->checksum = 0;
    ipv4->checksum = calculate_checksum(ipv4, sizeof(struct ipv4_header));
    
    if (payload && payload_len > 0) {
        kmemcpy(buffer + sizeof(struct ipv4_header), payload, payload_len);
    }
    
    /* TODO: Routing table to find correct gateway MAC or broadcast */
    /* For now, just broadcast everything for simplicity (especially DHCP) */
    uint8_t dst_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    ethernet_send(dst_mac, ETHERTYPE_IPV4, buffer, total_len);
}