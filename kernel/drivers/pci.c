/*
 * kernel/drivers/pci.c
 * PCI Bus enumeration and device discovery.
 */
#include "../include/pci.h"
#include "../include/kprintf.h"
#include "../include/net.h"

net_device_t g_net_dev = {0};

/* Read a 16-bit word from the PCI configuration space */
uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;

    /* Create configuration address */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    /* Write out the address */
    outl(PCI_CONFIG_ADDRESS, address);

    /* Read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32-bit register */
    tmp = (uint16_t)((inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

/* Read a 32-bit dword from the PCI configuration space */
uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void pci_config_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

/* Helper to get device class */
static uint16_t pci_get_vendor_id(uint8_t bus, uint8_t slot, uint8_t func) {
    return pci_config_read_word(bus, slot, func, 0x00);
}

static void check_device(uint8_t bus, uint8_t slot, uint8_t func) {
    uint16_t vendor_id = pci_get_vendor_id(bus, slot, func);
    if (vendor_id == 0xFFFF) return; /* Device doesn't exist */
    
    uint16_t device_id = pci_config_read_word(bus, slot, func, 0x02);
    
    uint16_t class_subclass = pci_config_read_word(bus, slot, func, 0x0A);
    uint8_t class_code = (class_subclass >> 8) & 0xFF;
    uint8_t subclass = class_subclass & 0xFF;
    
    uint16_t prog_rev = pci_config_read_word(bus, slot, func, 0x08);
    uint8_t prog_if = (prog_rev >> 8) & 0xFF;
    
    uint16_t hdr_type = pci_config_read_word(bus, slot, func, 0x0E);
    uint8_t header_type = hdr_type & 0xFF;

    /* Print out found device */
    kprintf("[PCI] Bus %d, Slot %d, Func %d: Vendor %x, Device %x [Class %x:%x]\n",
            bus, slot, func, vendor_id, device_id, class_code, subclass);
            
    /* Check if it's a Network Controller */
    if (class_code == PCI_CLASS_NETWORK && subclass == PCI_SUBCLASS_ETHERNET) {
        kprintf("      -> FOUND ETHERNET CONTROLLER! (Vendor: 0x%x, Dev: 0x%x)\n", vendor_id, device_id);
        
        /* Store it globally so ipconfig can see it */
        if (!g_net_dev.present) {
            g_net_dev.present = true;
            g_net_dev.vendor_id = vendor_id;
            g_net_dev.device_id = device_id;
            
            if (vendor_id == 0x8086 && device_id == 0x100E) {
                g_net_dev.name = "Intel PRO/1000 (e1000)";
            } else if (vendor_id == 0x10EC && device_id == 0x8139) {
                g_net_dev.name = "Realtek RTL8139";
            } else if (vendor_id == 0x1022 && device_id == 0x2000) {
                g_net_dev.name = "AMD PCnet-FAST III (Am79C973)";
            } else {
                g_net_dev.name = "Unknown Generic NIC";
            }
            
            /* Fill with fake MAC/IP for now just for UI purposes until driver is complete */
            g_net_dev.mac[0] = 0x52; g_net_dev.mac[1] = 0x54; g_net_dev.mac[2] = 0x00;
            g_net_dev.mac[3] = 0x12; g_net_dev.mac[4] = 0x34; g_net_dev.mac[5] = 0x56;
            
            g_net_dev.ip[0] = 0; g_net_dev.ip[1] = 0; g_net_dev.ip[2] = 0; g_net_dev.ip[3] = 0;
            g_net_dev.mask[0] = 0; g_net_dev.mask[1] = 0; g_net_dev.mask[2] = 0; g_net_dev.mask[3] = 0;
            g_net_dev.gateway[0] = 0; g_net_dev.gateway[1] = 0; g_net_dev.gateway[2] = 0; g_net_dev.gateway[3] = 0;
        }
    }
}

void pci_init(void) {
    kprintf("\n--- PCI Enumeration ---\n");
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint16_t vendor_id = pci_get_vendor_id((uint8_t)bus, slot, 0);
            if (vendor_id == 0xFFFF) continue; /* Empty slot */
            
            /* Check function 0 */
            check_device((uint8_t)bus, slot, 0);
            
            /* Check if multi-function device */
            uint16_t hdr_type = pci_config_read_word((uint8_t)bus, slot, 0, 0x0E);
            if ((hdr_type & 0x80) != 0) {
                /* It is a multi-function device, check remaining functions */
                for (uint8_t func = 1; func < 8; func++) {
                    if (pci_get_vendor_id((uint8_t)bus, slot, func) != 0xFFFF) {
                        check_device((uint8_t)bus, slot, func);
                    }
                }
            }
        }
    }
    kprintf("--- End of PCI ---\n\n");
}