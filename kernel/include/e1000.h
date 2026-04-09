/*
 * kernel/include/e1000.h
 * Intel PRO/1000 (e1000) Network Driver
 */
#ifndef E1000_H
#define E1000_H

#include "kernel.h"
#include "pci.h"

/* Registers */
#define E1000_REG_CTRL      0x0000
#define E1000_REG_STATUS    0x0008
#define E1000_REG_EEPROM    0x0014
#define E1000_REG_CTRL_EXT  0x0018
#define E1000_REG_ICR       0x00C0
#define E1000_REG_IMS       0x00D0
#define E1000_REG_IMC       0x00D8

#define E1000_REG_RCTRL     0x0100
#define E1000_REG_RXDESCLO  0x2800
#define E1000_REG_RXDESCHI  0x2804
#define E1000_REG_RXDESCLEN 0x2808
#define E1000_REG_RXDESCHEAD 0x2810
#define E1000_REG_RXDESCTAIL 0x2818

#define E1000_REG_TCTRL     0x0400
#define E1000_REG_TXDESCLO  0x3800
#define E1000_REG_TXDESCHI  0x3804
#define E1000_REG_TXDESCLEN 0x3808
#define E1000_REG_TXDESCHEAD 0x3810
#define E1000_REG_TXDESCTAIL 0x3818

#define E1000_REG_MTA       0x5200

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

struct e1000_rx_desc {
    uint64_t addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
} __attribute__((packed));

struct e1000_tx_desc {
    uint64_t addr;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
} __attribute__((packed));

void e1000_init(uint8_t bus, uint8_t slot, uint8_t func);
int e1000_send_packet(const void *data, uint16_t len);

#endif /* E1000_H */