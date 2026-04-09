/*
 * kernel/drivers/e1000.c
 * Intel PRO/1000 Network Driver
 */
#include "../include/e1000.h"
#include "../include/kprintf.h"
#include "../include/net.h"
#include "../mm/vmm.h"
#include "../mm/heap.h"
#include "../arch/x86_64/irq.h"
#include "../net/ethernet.h"

static uint8_t *s_e1000_mmio;
static struct e1000_rx_desc *s_rx_descs;
static struct e1000_tx_desc *s_tx_descs;
static uint16_t s_rx_cur = 0;
static uint16_t s_tx_cur = 0;
static uint8_t  s_e1000_irq = 0;

static void e1000_write_reg(uint16_t reg, uint32_t val) {
    *(volatile uint32_t *)(s_e1000_mmio + reg) = val;
}

static uint32_t e1000_read_reg(uint16_t reg) {
    return *(volatile uint32_t *)(s_e1000_mmio + reg);
}

static void e1000_read_mac(void) {
    /* Read MAC from EEPROM or direct registers. In QEMU, registers 0x5400 and 0x5404 usually work. */
    uint32_t mac_low = e1000_read_reg(0x5400);
    uint32_t mac_high = e1000_read_reg(0x5404);
    
    g_net_dev.mac[0] = mac_low & 0xFF;
    g_net_dev.mac[1] = (mac_low >> 8) & 0xFF;
    g_net_dev.mac[2] = (mac_low >> 16) & 0xFF;
    g_net_dev.mac[3] = (mac_low >> 24) & 0xFF;
    g_net_dev.mac[4] = mac_high & 0xFF;
    g_net_dev.mac[5] = (mac_high >> 8) & 0xFF;
    
    kprintf("  [E1000] MAC Address: %X-%X-%X-%X-%X-%X\n", 
            g_net_dev.mac[0], g_net_dev.mac[1], g_net_dev.mac[2], 
            g_net_dev.mac[3], g_net_dev.mac[4], g_net_dev.mac[5]);
}

static void e1000_init_rx(void) {
    uint32_t size = sizeof(struct e1000_rx_desc) * E1000_NUM_RX_DESC;
    s_rx_descs = (struct e1000_rx_desc *)kmalloc_aligned(size, 16);
    
    for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
        s_rx_descs[i].addr = (uint64_t)(uintptr_t)kmalloc_aligned(2048, 16);
        s_rx_descs[i].status = 0;
    }
    
    e1000_write_reg(E1000_REG_RXDESCLO, (uint32_t)((uint64_t)s_rx_descs & 0xFFFFFFFF));
    e1000_write_reg(E1000_REG_RXDESCHI, (uint32_t)(((uint64_t)s_rx_descs >> 32) & 0xFFFFFFFF));
    e1000_write_reg(E1000_REG_RXDESCLEN, size);
    e1000_write_reg(E1000_REG_RXDESCHEAD, 0);
    e1000_write_reg(E1000_REG_RXDESCTAIL, E1000_NUM_RX_DESC - 1);
    
    /* RCTRL: EN (bit 1), MPE (bit 4), BAM (bit 15), BSIZE=2048 (bit 16=0, bit 17=0) */
    uint32_t rctrl = (1 << 1) | (1 << 4) | (1 << 15);
    e1000_write_reg(E1000_REG_RCTRL, rctrl);
}

static void e1000_init_tx(void) {
    uint32_t size = sizeof(struct e1000_tx_desc) * E1000_NUM_TX_DESC;
    s_tx_descs = (struct e1000_tx_desc *)kmalloc_aligned(size, 16);
    
    for (int i = 0; i < E1000_NUM_TX_DESC; i++) {
        s_tx_descs[i].addr = 0;
        s_tx_descs[i].cmd = 0;
        s_tx_descs[i].status = 1; /* DD (Descriptor Done) */
    }
    
    e1000_write_reg(E1000_REG_TXDESCLO, (uint32_t)((uint64_t)s_tx_descs & 0xFFFFFFFF));
    e1000_write_reg(E1000_REG_TXDESCHI, (uint32_t)(((uint64_t)s_tx_descs >> 32) & 0xFFFFFFFF));
    e1000_write_reg(E1000_REG_TXDESCLEN, size);
    e1000_write_reg(E1000_REG_TXDESCHEAD, 0);
    e1000_write_reg(E1000_REG_TXDESCTAIL, 0);
    
    /* TCTRL: EN (bit 1), PSP (bit 3), CT=15 (bit 4), COLD=64 (bit 12) */
    uint32_t tctrl = (1 << 1) | (1 << 3) | (15 << 4) | (64 << 12);
    e1000_write_reg(E1000_REG_TCTRL, tctrl);
}

int e1000_send_packet(const void *data, uint16_t len) {
    s_tx_descs[s_tx_cur].addr = (uint64_t)(uintptr_t)data;
    s_tx_descs[s_tx_cur].length = len;
    /* CMD: EOP (bit 0), IFCS (bit 1), RS (bit 3) */
    s_tx_descs[s_tx_cur].cmd = (1 << 0) | (1 << 1) | (1 << 3);
    s_tx_descs[s_tx_cur].status = 0;
    
    uint8_t old_cur = s_tx_cur;
    s_tx_cur = (s_tx_cur + 1) % E1000_NUM_TX_DESC;
    
    e1000_write_reg(E1000_REG_TXDESCTAIL, s_tx_cur);
    
    /* Wait for transmission to finish (DD bit in status) */
    while (!(s_tx_descs[old_cur].status & 0xFF)) {
        /* Busy wait */
    }
    return 0;
}

static void e1000_irq_handler(registers_t *regs) {
    (void)regs;
    
    /* Read Interrupt Cause Read to find out what happened */
    uint32_t status = e1000_read_reg(E1000_REG_ICR);
    
    /* Check if it was a Receiver Timer Interrupt (Packet arrived) */
    if (status & 0x80) { /* RXT0 = bit 7 */
        /* Poll the RX ring until we process all received packets */
        while ((s_rx_descs[s_rx_cur].status & 0x1)) { /* DD bit = bit 0 */
            uint8_t *pkt = (uint8_t *)(uintptr_t)s_rx_descs[s_rx_cur].addr;
            uint16_t len = s_rx_descs[s_rx_cur].length;
            
            kprintf("  [E1000] Packet received! Length: %d bytes\n", len);
            
            /* Send packet to network stack (Ethernet layer) */
            ethernet_receive(pkt, len);
            
            /* Tell GUI to redraw if terminal was updated */
            extern volatile bool g_gui_needs_update;
            g_gui_needs_update = true;
            
            /* Reset descriptor for next packet */
            s_rx_descs[s_rx_cur].status = 0;
            uint16_t old_cur = s_rx_cur;
            s_rx_cur = (s_rx_cur + 1) % E1000_NUM_RX_DESC;
            
            /* Update hardware tail pointer */
            e1000_write_reg(E1000_REG_RXDESCTAIL, old_cur);
        }
    }
    
    /* Acknowledge interrupt to the PIC */
    irq_send_eoi(s_e1000_irq);
}

void e1000_init(uint8_t bus, uint8_t slot, uint8_t func) {
    kprintf("  [E1000] Initializing driver...\n");
    
    /* 1. Read BAR0 (Memory Mapped I/O base address) */
    uint32_t bar0 = pci_config_read_dword(bus, slot, func, 0x10);
    
    if (bar0 & 1) {
        kprintf("  [E1000] Error: BAR0 is I/O mapped, expected MMIO.\n");
        return;
    }
    
    uint64_t mmio_phys = bar0 & ~0xF;
    
    /* Ensure the memory is mapped (assuming identity map or we map it) */
    /* Map 2MB just to be safe for the device memory */
    for (uint64_t off = 0; off < 0x200000; off += 4096) {
        vmm_map(mmio_phys + off, mmio_phys + off, VMM_PRESENT | VMM_WRITABLE);
    }
    
    s_e1000_mmio = (uint8_t *)(uintptr_t)mmio_phys;
    kprintf("  [E1000] MMIO mapped at: %p\n", s_e1000_mmio);
    
    /* Enable Bus Mastering (Bit 2) in PCI Command Register so device can perform DMA */
    uint32_t pci_cmd = pci_config_read_word(bus, slot, func, 0x04);
    pci_config_write_dword(bus, slot, func, 0x04, pci_cmd | 0x04);

    e1000_read_mac();
    
    /* Initialize RX/TX rings */
    e1000_init_rx();
    e1000_init_tx();
    
    /* Enable interrupts (clear mask and then set desired) */
    e1000_write_reg(E1000_REG_IMC, 0xFFFFFFFF); /* IMC: disable all */
    e1000_write_reg(E1000_REG_IMS, (1 << 7));   /* IMS: RXT0 (Receiver Timer Interrupt) */
    
    /* Read interrupt line from PCI */
    s_e1000_irq = pci_config_read_dword(bus, slot, func, 0x3C) & 0xFF;
    kprintf("  [E1000] Registered on IRQ %d\n", s_e1000_irq);
    
    /* Register handler and unmask PIC */
    irq_register_handler(s_e1000_irq, e1000_irq_handler);
    irq_unmask(s_e1000_irq);
    
    kprintf("  [E1000] Driver init OK (RX/TX rings configured)...\n");
}
