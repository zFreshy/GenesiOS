#include "dns.h"
#include "udp.h"
#include "../include/kprintf.h"
#include "../include/net.h"

static inline uint16_t htons(uint16_t hostshort) {
    return (hostshort >> 8) | (hostshort << 8);
}
static inline uint16_t ntohs(uint16_t netshort) {
    return htons(netshort);
}

/* Hardcoded transaction ID for now */
#define DNS_TRANSACTION_ID 0x1234

static char s_last_queried_domain[128] = {0};
static uint8_t s_resolved_ip[4] = {0};
static bool s_dns_resolved = false;

/* Returns a pointer to the resolved IP, or NULL if not ready/failed. Non-blocking. */
uint8_t* dns_get_resolved_ip(void) {
    if (s_dns_resolved) return s_resolved_ip;
    return NULL;
}

static void encode_domain_name(uint8_t *dst, const char *domain) {
    int lock = 0;
    for (int i = 0; domain[i]; i++) {
        if (domain[i] == '.') {
            *dst++ = i - lock;
            for (int j = lock; j < i; j++) {
                *dst++ = domain[j];
            }
            lock = i + 1;
        }
    }
    
    /* Last part */
    int i = kstrlen(domain);
    *dst++ = i - lock;
    for (int j = lock; j < i; j++) {
        *dst++ = domain[j];
    }
    *dst++ = 0; /* Null terminator */
}

void dns_resolve(const char *domain) {
    if (!g_net_dev.present || g_net_dev.ip[0] == 0) {
        kprintf("  [DNS] Network not ready.\n");
        return;
    }
    
    /* Copy domain to last queried */
    int len = kstrlen(domain);
    if (len > 127) len = 127;
    kmemcpy(s_last_queried_domain, domain, len);
    s_last_queried_domain[len] = '\0';
    
    s_dns_resolved = false;
    kmemset(s_resolved_ip, 0, 4);
    
    uint8_t packet[512];
    kmemset(packet, 0, 512);
    
    struct dns_header *hdr = (struct dns_header *)packet;
    hdr->id = htons(DNS_TRANSACTION_ID);
    hdr->flags = htons(0x0100); /* Standard query, Recursion Desired */
    hdr->qdcount = htons(1);    /* 1 question */
    hdr->ancount = 0;
    hdr->nscount = 0;
    hdr->arcount = 0;
    
    uint8_t *qname = packet + sizeof(struct dns_header);
    encode_domain_name(qname, domain);
    
    uint8_t *qinfo = qname + kstrlen((const char *)qname) + 1;
    
    /* QTYPE = 1 (A record - IPv4) */
    qinfo[0] = 0; qinfo[1] = 1;
    /* QCLASS = 1 (IN - Internet) */
    qinfo[2] = 0; qinfo[3] = 1;
    
    uint16_t packet_len = sizeof(struct dns_header) + (qinfo - qname) + 4;
    
    /* Ensure DNS is not 0.0.0.0 */
    if (g_net_dev.dns[0] == 0) {
        g_net_dev.dns[0] = 8; g_net_dev.dns[1] = 8; g_net_dev.dns[2] = 8; g_net_dev.dns[3] = 8;
    }
    
    /* Hack for QEMU: Sometimes the DHCP assigns 0.0.0.0, or QEMU user net 
     * DNS is at 10.0.2.3 and gateway at 10.0.2.2. If we are trying to resolve
     * and we are on 10.0.2.x subnet but our DNS is 0.0.0.0 or 8.8.8.8, 
     * use 10.0.2.3 to be safe inside QEMU NAT. */
    if (g_net_dev.ip[0] == 10 && g_net_dev.ip[1] == 0 && g_net_dev.ip[2] == 2) {
        g_net_dev.dns[0] = 10; g_net_dev.dns[1] = 0; g_net_dev.dns[2] = 2; g_net_dev.dns[3] = 3;
    }
    
    kprintf("  [DNS] Resolving %s via %d.%d.%d.%d...\n", 
            domain, g_net_dev.dns[0], g_net_dev.dns[1], g_net_dev.dns[2], g_net_dev.dns[3]);
            
    udp_send(g_net_dev.dns, 53535, 53, packet, packet_len);
}

void dns_receive(uint8_t *packet, uint16_t len) {
    if (len < sizeof(struct dns_header)) return;
    
    struct dns_header *hdr = (struct dns_header *)packet;
    if (ntohs(hdr->id) != DNS_TRANSACTION_ID) return;
    
    uint16_t flags = ntohs(hdr->flags);
    if ((flags & 0x8000) == 0) return; /* Not a response */
    
    uint16_t ancount = ntohs(hdr->ancount);
    if (ancount == 0) {
        kprintf("  [DNS] Could not resolve %s.\n", s_last_queried_domain);
        return;
    }
    
    /* Skip the Question section */
    uint8_t *ptr = packet + sizeof(struct dns_header);
    uint16_t qdcount = ntohs(hdr->qdcount);
    
    for (int i = 0; i < qdcount; i++) {
        /* Skip Name */
        while (*ptr != 0 && (*ptr & 0xC0) != 0xC0) {
            ptr += *ptr + 1;
        }
        if ((*ptr & 0xC0) == 0xC0) {
            ptr += 2; /* Compressed name */
        } else {
            ptr += 1; /* Null byte */
        }
        ptr += 4; /* QTYPE and QCLASS */
    }
    
    /* Parse the Answers section */
    for (int i = 0; i < ancount; i++) {
        /* Skip Name */
        if ((*ptr & 0xC0) == 0xC0) {
            ptr += 2;
        } else {
            while (*ptr != 0) ptr += *ptr + 1;
            ptr += 1;
        }
        
        uint16_t atype = (ptr[0] << 8) | ptr[1]; ptr += 2;
        uint16_t aclass = (ptr[0] << 8) | ptr[1]; ptr += 2;
        ptr += 4; /* TTL */
        uint16_t rdlength = (ptr[0] << 8) | ptr[1]; ptr += 2;
        
        if (atype == 1 && aclass == 1 && rdlength == 4) { /* A record (IPv4) */
            s_resolved_ip[0] = ptr[0];
            s_resolved_ip[1] = ptr[1];
            s_resolved_ip[2] = ptr[2];
            s_resolved_ip[3] = ptr[3];
            s_dns_resolved = true;
            
            kprintf("  [DNS] %s is at %d.%d.%d.%d\n", 
                    s_last_queried_domain, 
                    s_resolved_ip[0], s_resolved_ip[1], s_resolved_ip[2], s_resolved_ip[3]);
                    
            /* Tell GUI to redraw */
            extern volatile bool g_gui_needs_update;
            g_gui_needs_update = true;
            return;
        }
        
        ptr += rdlength;
    }
}