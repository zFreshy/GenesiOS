#ifndef TCP_H
#define TCP_H

#include "../include/kernel.h"

struct tcp_header {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq;
    uint32_t ack;
    uint8_t  data_offset; // 4 bits offset, 4 bits reserved
    uint8_t  flags;
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_ptr;
} __attribute__((packed));

void tcp_init(void);
void tcp_receive(uint8_t *packet, uint16_t len, uint8_t *src_ip);
int tcp_connect(uint8_t *dst_ip, uint16_t dst_port);
void tcp_send_data(int sock, uint8_t *data, uint16_t len);
void tcp_close(int sock);
int tcp_get_rx_len(void);
char* tcp_get_rx_buffer(void);

#endif