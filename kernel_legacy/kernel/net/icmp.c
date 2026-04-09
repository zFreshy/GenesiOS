#include "icmp.h"
#include "../include/kprintf.h"

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

void icmp_receive(uint8_t *packet, uint16_t len, uint8_t *src_ip) {
    if (len < sizeof(struct icmp_header)) return;
    
    struct icmp_header *icmp = (struct icmp_header *)packet;
    
    if (icmp->type == ICMP_ECHO_REQUEST) {
        kprintf("  [ICMP] Ping request from %d.%d.%d.%d. Sending reply...\n",
                src_ip[0], src_ip[1], src_ip[2], src_ip[3]);
        
        uint16_t payload_len = len - sizeof(struct icmp_header);
        uint8_t *payload = packet + sizeof(struct icmp_header);
        
        icmp_send_reply(src_ip, ntohs(icmp->id), ntohs(icmp->sequence), payload, payload_len);
    } else if (icmp->type == ICMP_ECHO_REPLY) {
        kprintf("  [ICMP] Ping reply from %d.%d.%d.%d (seq=%d)\n",
                src_ip[0], src_ip[1], src_ip[2], src_ip[3], ntohs(icmp->sequence));
    }
}

void icmp_send_reply(uint8_t *dst_ip, uint16_t id, uint16_t seq, uint8_t *payload, uint16_t payload_len) {
    uint16_t total_len = sizeof(struct icmp_header) + payload_len;
    extern void *kmalloc(size_t size);
    extern void kfree(void *ptr);
    uint8_t *buffer = (uint8_t *)kmalloc(total_len);
    if (!buffer) return;
    kmemset(buffer, 0, total_len);
    
    struct icmp_header *icmp = (struct icmp_header *)buffer;
    icmp->type = ICMP_ECHO_REPLY;
    icmp->code = 0;
    icmp->id = htons(id);
    icmp->sequence = htons(seq);
    
    if (payload && payload_len > 0) {
        kmemcpy(buffer + sizeof(struct icmp_header), payload, payload_len);
    }
    
    icmp->checksum = 0;
    icmp->checksum = calculate_checksum(buffer, total_len);
    
    ipv4_send(dst_ip, 1 /* ICMP */, buffer, total_len);
    kfree(buffer);
}

void icmp_send_request(uint8_t *dst_ip, uint16_t id, uint16_t seq, uint8_t *payload, uint16_t payload_len) {
    uint16_t total_len = sizeof(struct icmp_header) + payload_len;
    extern void *kmalloc(size_t size);
    extern void kfree(void *ptr);
    uint8_t *buffer = (uint8_t *)kmalloc(total_len);
    if (!buffer) return;
    kmemset(buffer, 0, total_len);
    
    struct icmp_header *icmp = (struct icmp_header *)buffer;
    icmp->type = ICMP_ECHO_REQUEST;
    icmp->code = 0;
    icmp->id = htons(id);
    icmp->sequence = htons(seq);
    
    if (payload && payload_len > 0) {
        kmemcpy(buffer + sizeof(struct icmp_header), payload, payload_len);
    }
    
    icmp->checksum = 0;
    icmp->checksum = calculate_checksum(buffer, total_len);
    
    ipv4_send(dst_ip, 1 /* ICMP */, buffer, total_len);
    kfree(buffer);
}