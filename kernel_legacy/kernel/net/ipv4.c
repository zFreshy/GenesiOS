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
    } else if (ipv4->protocol == 6) { /* TCP */
        extern void tcp_receive(uint8_t *packet, uint16_t len, uint8_t *src_ip);
        tcp_receive(payload, payload_len, ipv4->src_ip);
    }
}

void ipv4_send(uint8_t *dst_ip, uint8_t protocol, uint8_t *payload, uint16_t payload_len) {
    uint16_t total_len = sizeof(struct ipv4_header) + payload_len;
    
    extern void *kmalloc(size_t size);
    extern void kfree(void *ptr);
    uint8_t *buffer = (uint8_t *)kmalloc(total_len);
    if (!buffer) return;
    kmemset(buffer, 0, total_len);
    
    struct ipv4_header *ipv4 = (struct ipv4_header *)buffer;
    ipv4->version = 4;
    ipv4->ihl = 5;
    ipv4->tos = 0;
    ipv4->length = htons(total_len);
    ipv4->id = 0;
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
    
    /* Routing: Use gateway MAC if we have it, else broadcast and request ARP */
    uint8_t dst_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    bool is_broadcast = (dst_ip[0] == 255 && dst_ip[1] == 255 && dst_ip[2] == 255 && dst_ip[3] == 255);
    
    if (!is_broadcast) {
        extern uint8_t g_gateway_mac[6];
        bool has_gw = false;
        for (int i = 0; i < 6; i++) {
            if (g_gateway_mac[i] != 0) has_gw = true;
        }
        
        /* If we are talking to a local subnet IP, we should ARP that IP directly, not the gateway.
         * But for simplicity right now, if it's not the gateway IP, we just send to gateway MAC.
         * If we don't have gateway MAC, we ARP the gateway. */
        
        if (has_gw) {
            for (int i = 0; i < 6; i++) dst_mac[i] = g_gateway_mac[i];
        } else {
            /* Fallback to Google DNS if gateway is 0.0.0.0 */
            if (g_net_dev.gateway[0] == 0) {
                g_net_dev.gateway[0] = 10; g_net_dev.gateway[1] = 0; g_net_dev.gateway[2] = 2; g_net_dev.gateway[3] = 2; // Default QEMU router
            }
            
            extern void arp_send_request(uint8_t *target_ip);
            arp_send_request(g_net_dev.gateway);
            kprintf("  [Net] Waiting for ARP to resolve gateway MAC... (Please retry ping in a moment)\n");
            /* Still send it as broadcast, it might drop, but next time it will work */
        }
    }
    
    ethernet_send(dst_mac, ETHERTYPE_IPV4, buffer, total_len);
    kfree(buffer);
}