#include "udp.h"
#include "ipv4.h"
#include "../include/kprintf.h"

static inline uint16_t htons(uint16_t hostshort) {
    return (hostshort >> 8) | (hostshort << 8);
}
static inline uint16_t ntohs(uint16_t netshort) {
    return htons(netshort);
}

void udp_receive(uint8_t *packet, uint16_t len, uint8_t *src_ip) {
    if (len < sizeof(struct udp_header)) return;
    
    struct udp_header *udp = (struct udp_header *)packet;
    uint16_t src_port = ntohs(udp->src_port);
    uint16_t dst_port = ntohs(udp->dst_port);
    uint16_t udp_len = ntohs(udp->length);
    
    if (len < udp_len) return; // truncated packet
    
    uint8_t *payload = packet + sizeof(struct udp_header);
    uint16_t payload_len = udp_len - sizeof(struct udp_header);
    
    kprintf("  [UDP] Received packet on port %d from %d.%d.%d.%d:%d\n",
            dst_port, src_ip[0], src_ip[1], src_ip[2], src_ip[3], src_port);
            
    if (dst_port == 68) { /* DHCP Client port */
        extern void dhcp_receive(uint8_t *packet, uint16_t len);
        dhcp_receive(payload, payload_len);
    } else if (dst_port == 53535) { /* DNS Client port */
        extern void dns_receive(uint8_t *packet, uint16_t len);
        dns_receive(payload, payload_len);
    }
}

void udp_send(uint8_t *dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t *payload, uint16_t payload_len) {
    uint16_t total_len = sizeof(struct udp_header) + payload_len;
    
    uint8_t buffer[2048];
    kmemset(buffer, 0, total_len);
    
    struct udp_header *udp = (struct udp_header *)buffer;
    udp->src_port = htons(src_port);
    udp->dst_port = htons(dst_port);
    udp->length = htons(total_len);
    udp->checksum = 0; /* Optional in IPv4 */
    
    if (payload && payload_len > 0) {
        kmemcpy(buffer + sizeof(struct udp_header), payload, payload_len);
    }
    
    ipv4_send(dst_ip, 17, buffer, total_len);
}