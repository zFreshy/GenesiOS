#include "dhcp.h"
#include "udp.h"
#include "../include/kprintf.h"
#include "../include/net.h"

#define DHCP_MAGIC_COOKIE 0x63825363

static uint32_t s_dhcp_xid = 0x12345678; // Hardcoded transaction ID for now

void dhcp_init(void) {
    kprintf("  [DHCP] Initializing client...\n");
    /* Reset current IP */
    for (int i = 0; i < 4; i++) {
        g_net_dev.ip[i] = 0;
        g_net_dev.mask[i] = 0;
        g_net_dev.gateway[i] = 0;
    }
    dhcp_send_discover();
}

void dhcp_send_discover(void) {
    struct dhcp_packet dhcp;
    kmemset(&dhcp, 0, sizeof(struct dhcp_packet));
    
    dhcp.op = 1; /* BOOTREQUEST */
    dhcp.htype = 1; /* Ethernet */
    dhcp.hlen = 6;
    dhcp.hops = 0;
    dhcp.xid = s_dhcp_xid;
    dhcp.secs = 0;
    dhcp.flags = 0; /* Unicast response is fine, or broadcast 0x8000 */
    
    for (int i = 0; i < 6; i++) dhcp.chaddr[i] = g_net_dev.mac[i];
    
    dhcp.magic_cookie = DHCP_MAGIC_COOKIE; /* Need to be big-endian on wire, but it's symmetric in some ways? Wait, no, it's 99 130 83 99 */
    /* 0x63 0x82 0x53 0x63 */
    uint8_t *cookie = (uint8_t *)&dhcp.magic_cookie;
    cookie[0] = 0x63; cookie[1] = 0x82; cookie[2] = 0x53; cookie[3] = 0x63;
    
    /* Options */
    uint8_t *opt = dhcp.options;
    
    /* Option 53: DHCP Message Type (Discover = 1) */
    *opt++ = 53; *opt++ = 1; *opt++ = 1;
    
    /* Option 55: Parameter Request List */
    *opt++ = 55; *opt++ = 3;
    *opt++ = 1; /* Subnet mask */
    *opt++ = 3; /* Router */
    *opt++ = 6; /* DNS */
    
    /* End Option */
    *opt++ = 255;
    
    uint16_t total_len = sizeof(struct dhcp_packet);
    
    uint8_t broadcast_ip[4] = {255, 255, 255, 255};
    udp_send(broadcast_ip, 68, 67, (uint8_t *)&dhcp, total_len);
    
    kprintf("  [DHCP] Sent DHCP Discover\n");
}

void dhcp_send_request(uint8_t *offered_ip, uint8_t *server_ip) {
    struct dhcp_packet dhcp;
    kmemset(&dhcp, 0, sizeof(struct dhcp_packet));
    
    dhcp.op = 1; /* BOOTREQUEST */
    dhcp.htype = 1;
    dhcp.hlen = 6;
    dhcp.xid = s_dhcp_xid;
    
    for (int i = 0; i < 6; i++) dhcp.chaddr[i] = g_net_dev.mac[i];
    
    uint8_t *cookie = (uint8_t *)&dhcp.magic_cookie;
    cookie[0] = 0x63; cookie[1] = 0x82; cookie[2] = 0x53; cookie[3] = 0x63;
    
    uint8_t *opt = dhcp.options;
    
    /* Option 53: DHCP Message Type (Request = 3) */
    *opt++ = 53; *opt++ = 1; *opt++ = 3;
    
    /* Option 50: Requested IP Address */
    *opt++ = 50; *opt++ = 4;
    for (int i=0; i<4; i++) *opt++ = offered_ip[i];
    
    /* Option 54: Server Identifier */
    *opt++ = 54; *opt++ = 4;
    for (int i=0; i<4; i++) *opt++ = server_ip[i];
    
    *opt++ = 255; /* End */
    
    uint16_t total_len = sizeof(struct dhcp_packet);
    uint8_t broadcast_ip[4] = {255, 255, 255, 255};
    udp_send(broadcast_ip, 68, 67, (uint8_t *)&dhcp, total_len);
    
    kprintf("  [DHCP] Sent DHCP Request for %d.%d.%d.%d\n", offered_ip[0], offered_ip[1], offered_ip[2], offered_ip[3]);
}

void dhcp_receive(uint8_t *packet, uint16_t len) {
    if (len < sizeof(struct dhcp_packet)) return;
    
    struct dhcp_packet *dhcp = (struct dhcp_packet *)packet;
    if (dhcp->op != 2) return; /* Not a BOOTREPLY */
    if (dhcp->xid != s_dhcp_xid) return; /* Not our transaction */
    
    uint8_t *opt = dhcp->options;
    uint8_t msg_type = 0;
    uint8_t server_ip[4] = {0};
    uint8_t mask[4] = {0};
    uint8_t router[4] = {0};
    uint8_t dns[4] = {0};
    
    while (*opt != 255 && (uintptr_t)opt < (uintptr_t)packet + len) {
        uint8_t type = *opt++;
        if (type == 0) continue; /* Pad */
        uint8_t length = *opt++;
        
        if (type == 53) { /* DHCP Message Type */
            msg_type = *opt;
        } else if (type == 54) { /* Server Identifier */
            for (int i=0; i<4; i++) server_ip[i] = opt[i];
        } else if (type == 1) { /* Subnet Mask */
            for (int i=0; i<4; i++) mask[i] = opt[i];
        } else if (type == 3) { /* Router */
            for (int i=0; i<4; i++) router[i] = opt[i];
        } else if (type == 6) { /* DNS Server */
            for (int i=0; i<4; i++) dns[i] = opt[i]; /* Just taking the primary DNS */
        }
        
        opt += length;
    }
    
    if (msg_type == 2) { /* DHCP Offer */
        kprintf("  [DHCP] Received DHCP Offer: %d.%d.%d.%d\n", dhcp->yiaddr[0], dhcp->yiaddr[1], dhcp->yiaddr[2], dhcp->yiaddr[3]);
        dhcp_send_request(dhcp->yiaddr, server_ip);
    } else if (msg_type == 5) { /* DHCP ACK */
        kprintf("  [DHCP] Received DHCP ACK!\n");
        for (int i=0; i<4; i++) {
            g_net_dev.ip[i] = dhcp->yiaddr[i];
            g_net_dev.mask[i] = mask[i];
            g_net_dev.gateway[i] = router[i];
            g_net_dev.dns[i] = dns[i];
        }
        
        /* If no DNS was provided, fallback to Google DNS (8.8.8.8) */
        if (g_net_dev.dns[0] == 0) {
            g_net_dev.dns[0] = 8; g_net_dev.dns[1] = 8; g_net_dev.dns[2] = 8; g_net_dev.dns[3] = 8;
        }
        
        kprintf("  [DHCP] Bound to IP: %d.%d.%d.%d\n", g_net_dev.ip[0], g_net_dev.ip[1], g_net_dev.ip[2], g_net_dev.ip[3]);
        kprintf("  [DHCP] Gateway: %d.%d.%d.%d | DNS: %d.%d.%d.%d\n", 
                g_net_dev.gateway[0], g_net_dev.gateway[1], g_net_dev.gateway[2], g_net_dev.gateway[3],
                g_net_dev.dns[0], g_net_dev.dns[1], g_net_dev.dns[2], g_net_dev.dns[3]);
    }
}