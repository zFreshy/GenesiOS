#include "tcp.h"
#include "../include/kprintf.h"
#include "../include/net.h"

static inline uint16_t htons(uint16_t hostshort) {
    return (hostshort >> 8) | (hostshort << 8);
}
static inline uint16_t ntohs(uint16_t netshort) {
    return htons(netshort);
}
static inline uint32_t htonl(uint32_t hostlong) {
    return ((hostlong & 0xFF) << 24) | ((hostlong & 0xFF00) << 8) | ((hostlong & 0xFF0000) >> 8) | ((hostlong >> 24) & 0xFF);
}
static inline uint32_t ntohl(uint32_t netlong) {
    return htonl(netlong);
}

// Pseudo header for TCP checksum
struct tcp_pseudo_header {
    uint8_t src_ip[4];
    uint8_t dst_ip[4];
    uint8_t reserved;
    uint8_t protocol;
    uint16_t tcp_length;
} __attribute__((packed));

static uint16_t calculate_tcp_checksum(struct tcp_pseudo_header *phdr, uint8_t *tcp_pkt, uint16_t tcp_len) {
    extern void *kmalloc(size_t size);
    extern void kfree(void *ptr);
    int phdr_len = sizeof(struct tcp_pseudo_header);
    uint8_t *full = (uint8_t *)kmalloc(phdr_len + tcp_len);
    if (!full) return 0;

    kmemcpy(full, phdr, phdr_len);
    kmemcpy(full + phdr_len, tcp_pkt, tcp_len);
    
    uint32_t sum = 0;
    uint16_t *ptr = (uint16_t *)full;
    int count = phdr_len + tcp_len;
    
    while (count > 1) {
        sum += *ptr++;
        count -= 2;
    }
    if (count > 0) sum += *(uint8_t *)ptr;
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    uint16_t cs = ~sum;
    kfree(full);
    return cs;
}

// Very basic single-socket state machine
enum tcp_state {
    TCP_CLOSED,
    TCP_SYN_SENT,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT
};

struct tcp_socket {
    enum tcp_state state;
    uint8_t remote_ip[4];
    uint16_t remote_port;
    uint16_t local_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t rx_buffer[16384];
    uint16_t rx_len;
};

static struct tcp_socket g_sock = {0};

void tcp_init(void) {
    g_sock.state = TCP_CLOSED;
    g_sock.local_port = 49152; // Ephemeral port
}

int tcp_get_rx_len(void) {
    return g_sock.rx_len;
}

char* tcp_get_rx_buffer(void) {
    return (char*)g_sock.rx_buffer;
}

static void tcp_send_packet(uint8_t flags, uint8_t *data, uint16_t data_len) {
    int opt_len = (flags == 0x02) ? 4 : 0; // Add MSS option for SYN packets
    uint16_t total_len = sizeof(struct tcp_header) + opt_len + data_len;
    extern void *kmalloc(size_t size);
    extern void kfree(void *ptr);
    uint8_t *buffer = (uint8_t *)kmalloc(total_len);
    if (!buffer) return;
    kmemset(buffer, 0, total_len);

    struct tcp_header *tcp = (struct tcp_header *)buffer;
    tcp->src_port = htons(g_sock.local_port);
    tcp->dst_port = htons(g_sock.remote_port);
    tcp->seq = htonl(g_sock.seq_num);
    tcp->ack = htonl(g_sock.ack_num);
    
    /* tcp->data_offset is 4 bits length in dwords, shifted left by 4. */
    tcp->data_offset = ((sizeof(struct tcp_header) + opt_len) / 4) << 4;
    
    tcp->flags = flags;
    tcp->window_size = htons(8192);
    tcp->checksum = 0;
    tcp->urgent_ptr = 0;

    if (flags == 0x02) {
        uint8_t *opts = buffer + sizeof(struct tcp_header);
        opts[0] = 2; // MSS Kind
        opts[1] = 4; // MSS Length
        opts[2] = 0x05; // 1460 (0x05B4)
        opts[3] = 0xB4;
    }

    if (data && data_len > 0) {
        kmemcpy(buffer + sizeof(struct tcp_header) + opt_len, data, data_len);
    }

    struct tcp_pseudo_header phdr;
    for(int i=0; i<4; i++) {
        phdr.src_ip[i] = g_net_dev.ip[i];
        phdr.dst_ip[i] = g_sock.remote_ip[i];
    }
    phdr.reserved = 0;
    phdr.protocol = 6; // TCP
    phdr.tcp_length = htons(total_len);
    tcp->checksum = 0;
    tcp->checksum = calculate_tcp_checksum(&phdr, buffer, total_len);

    extern void ipv4_send(uint8_t *dst_ip, uint8_t protocol, uint8_t *payload, uint16_t payload_len);
    ipv4_send(g_sock.remote_ip, 6, buffer, total_len);
    kfree(buffer);
}

int tcp_connect(uint8_t *dst_ip, uint16_t dst_port) {
    if (g_sock.state != TCP_CLOSED) return -1;

    g_sock.state = TCP_SYN_SENT;
    for(int i=0; i<4; i++) g_sock.remote_ip[i] = dst_ip[i];
    g_sock.remote_port = dst_port;
    
    if (g_sock.local_port < 49152) {
        g_sock.local_port = 49152;
    }
    g_sock.local_port++;
    
    g_sock.seq_num = 0x12345678; // Random ISN
    g_sock.ack_num = 0;
    g_sock.rx_len = 0;
    kmemset(g_sock.rx_buffer, 0, sizeof(g_sock.rx_buffer));

    tcp_send_packet(0x02, NULL, 0); // SYN flag
    kprintf("  [TCP] SYN sent to %d.%d.%d.%d:%d from port %d\n", 
            dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3], dst_port, g_sock.local_port);
    
    // Wait for ESTABLISHED
    int timeout = 50; // 5 seconds roughly if each is 100ms
    extern void pit_sleep(uint64_t ms);
    while(g_sock.state != TCP_ESTABLISHED && timeout > 0) {
        pit_sleep(100);
        timeout--;
        if (timeout % 5 == 0 && g_sock.state == TCP_SYN_SENT) {
            tcp_send_packet(0x02, NULL, 0); /* Retry SYN */
        }
    }
    
    if (g_sock.state == TCP_ESTABLISHED) return 1;
    g_sock.state = TCP_CLOSED;
    return -1;
}

void tcp_send_data(int sock, uint8_t *data, uint16_t len) {
    if (g_sock.state != TCP_ESTABLISHED) return;
    
    tcp_send_packet(0x18, data, len); // PSH | ACK
    g_sock.seq_num += len;
}

void tcp_close(int sock) {
    if (g_sock.state == TCP_CLOSED) return;
    
    g_sock.state = TCP_FIN_WAIT;
    tcp_send_packet(0x11, NULL, 0); // FIN | ACK
    g_sock.seq_num++;
    
    int timeout = 10;
    extern void pit_sleep(uint64_t ms);
    while(g_sock.state != TCP_CLOSED && timeout > 0) {
        pit_sleep(100);
        timeout--;
    }
    g_sock.state = TCP_CLOSED;
}

void tcp_receive(uint8_t *packet, uint16_t len, uint8_t *src_ip) {
    if (len < sizeof(struct tcp_header)) return;
    
    struct tcp_header *tcp = (struct tcp_header *)packet;
    uint16_t src_port = ntohs(tcp->src_port);
    uint16_t dst_port = ntohs(tcp->dst_port);
    
    kprintf("  [TCP] Received packet on port %d from %d.%d.%d.%d:%d, flags=0x%x\n",
            dst_port, src_ip[0], src_ip[1], src_ip[2], src_ip[3], src_port, tcp->flags);
    
    if (dst_port != g_sock.local_port) return;
    
    uint32_t seq = ntohl(tcp->seq);
    uint32_t ack = ntohl(tcp->ack);
    uint8_t flags = tcp->flags;
    
    uint16_t header_len = (tcp->data_offset >> 4) * 4;
    uint8_t *payload = packet + header_len;
    uint16_t payload_len = len - header_len;
    
    if (g_sock.state == TCP_SYN_SENT) {
        if ((flags & 0x12) == 0x12) { // SYN-ACK
            g_sock.ack_num = seq + 1;
            g_sock.seq_num = ack;
            g_sock.state = TCP_ESTABLISHED;
            tcp_send_packet(0x10, NULL, 0); // ACK
            kprintf("  [TCP] Connection ESTABLISHED!\n");
        } else {
            /* Debug */
            kprintf("  [TCP] Received non SYN-ACK in SYN_SENT: flags=0x%x\n", flags);
        }
    } else if (g_sock.state == TCP_ESTABLISHED) {
        if (payload_len > 0) {
            // Append to rx_buffer
            if (g_sock.rx_len + payload_len < sizeof(g_sock.rx_buffer) - 1) {
                kmemcpy(g_sock.rx_buffer + g_sock.rx_len, payload, payload_len);
                g_sock.rx_len += payload_len;
                g_sock.rx_buffer[g_sock.rx_len] = '\0'; // Null terminate for printing
            }
            g_sock.ack_num = seq + payload_len;
            tcp_send_packet(0x10, NULL, 0); // ACK
        }
        
        if (flags & 0x01) { // FIN
            g_sock.ack_num = seq + 1;
            g_sock.state = TCP_CLOSED;
            tcp_send_packet(0x11, NULL, 0); // FIN-ACK
        }
    } else if (g_sock.state == TCP_FIN_WAIT) {
        if (flags & 0x01) { // FIN
            g_sock.ack_num = seq + 1;
            tcp_send_packet(0x10, NULL, 0); // ACK
            g_sock.state = TCP_CLOSED;
        } else if (flags & 0x10) { // ACK
            // If they ack our FIN, we might transition to closed if we received their FIN
            if (payload_len == 0 && !(flags & 0x01)) {
                // wait for FIN
            }
        }
    }
}