#include "arp.h"
#include "ethernet.h"
#include "../include/kprintf.h"
#include "../include/net.h"

static inline uint16_t htons(uint16_t hostshort) {
    return (hostshort >> 8) | (hostshort << 8);
}
static inline uint16_t ntohs(uint16_t netshort) {
    return htons(netshort);
}

uint8_t g_gateway_mac[6] = {0};

void arp_receive(uint8_t *packet, uint16_t len) {
    if (len < sizeof(struct arp_header)) return;
    
    struct arp_header *arp = (struct arp_header *)packet;
    
    if (ntohs(arp->hw_type) != ARP_HW_ETHERNET || ntohs(arp->proto_type) != ETHERTYPE_IPV4) {
        return; /* We only support Ethernet + IPv4 ARP */
    }
    
    if (ntohs(arp->opcode) == ARP_OP_REQUEST) {
        kprintf("  [ARP] Who has %d.%d.%d.%d? Tell %d.%d.%d.%d\n",
                arp->dst_ip[0], arp->dst_ip[1], arp->dst_ip[2], arp->dst_ip[3],
                arp->src_ip[0], arp->src_ip[1], arp->src_ip[2], arp->src_ip[3]);
                
        /* Compare with our IP */
        bool match = true;
        for (int i = 0; i < 4; i++) {
            if (arp->dst_ip[i] != g_net_dev.ip[i]) {
                match = false;
                break;
            }
        }
        
        if (match && g_net_dev.ip[0] != 0) {
            kprintf("  [ARP] That is us! Sending reply.\n");
            arp_send_reply(arp->src_mac, arp->src_ip);
        }
    } else if (ntohs(arp->opcode) == ARP_OP_REPLY) {
        kprintf("  [ARP] Reply: %d.%d.%d.%d is at %X:%X:%X:%X:%X:%X\n",
                arp->src_ip[0], arp->src_ip[1], arp->src_ip[2], arp->src_ip[3],
                arp->src_mac[0], arp->src_mac[1], arp->src_mac[2], 
                arp->src_mac[3], arp->src_mac[4], arp->src_mac[5]);
                
        /* Save gateway MAC if it matches */
        if (arp->src_ip[0] == g_net_dev.gateway[0] &&
            arp->src_ip[1] == g_net_dev.gateway[1] &&
            arp->src_ip[2] == g_net_dev.gateway[2] &&
            arp->src_ip[3] == g_net_dev.gateway[3]) {
            for (int i = 0; i < 6; i++) {
                g_gateway_mac[i] = arp->src_mac[i];
            }
            kprintf("  [ARP] Gateway MAC resolved.\n");
        } else {
            /* For QEMU's User Networking (SLIRP), the DNS server is often 10.0.2.3, 
             * while gateway is 10.0.2.2. If we just learned a MAC for a local IP,
             * and we don't have a gateway MAC, use it as a fallback gateway MAC. */
            for (int i = 0; i < 6; i++) {
                g_gateway_mac[i] = arp->src_mac[i];
            }
            kprintf("  [ARP] Stored MAC for %d.%d.%d.%d as route.\n", arp->src_ip[0], arp->src_ip[1], arp->src_ip[2], arp->src_ip[3]);
        }
    }
}

void arp_send_reply(uint8_t *dst_mac, uint8_t *dst_ip) {
    struct arp_header arp;
    arp.hw_type = htons(ARP_HW_ETHERNET);
    arp.proto_type = htons(ETHERTYPE_IPV4);
    arp.hw_len = 6;
    arp.proto_len = 4;
    arp.opcode = htons(ARP_OP_REPLY);
    
    for (int i = 0; i < 6; i++) {
        arp.src_mac[i] = g_net_dev.mac[i];
        arp.dst_mac[i] = dst_mac[i];
    }
    for (int i = 0; i < 4; i++) {
        arp.src_ip[i] = g_net_dev.ip[i];
        arp.dst_ip[i] = dst_ip[i];
    }
    
    ethernet_send(dst_mac, ETHERTYPE_ARP, (uint8_t *)&arp, sizeof(struct arp_header));
}

void arp_send_request(uint8_t *target_ip) {
    struct arp_header arp;
    arp.hw_type = htons(ARP_HW_ETHERNET);
    arp.proto_type = htons(ETHERTYPE_IPV4);
    arp.hw_len = 6;
    arp.proto_len = 4;
    arp.opcode = htons(ARP_OP_REQUEST);
    
    uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    for (int i = 0; i < 6; i++) {
        arp.src_mac[i] = g_net_dev.mac[i];
        arp.dst_mac[i] = 0x00; /* Target MAC is 0 in request */
    }
    for (int i = 0; i < 4; i++) {
        arp.src_ip[i] = g_net_dev.ip[i];
        arp.dst_ip[i] = target_ip[i];
    }
    
    ethernet_send(broadcast_mac, ETHERTYPE_ARP, (uint8_t *)&arp, sizeof(struct arp_header));
}