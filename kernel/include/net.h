/*
 * kernel/include/net.h
 * Network structures and definitions.
 */
#ifndef NET_H
#define NET_H

#include "kernel.h"

typedef struct {
    bool present;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t mac[6];
    uint8_t ip[4];
    uint8_t mask[4];
    uint8_t gateway[4];
    const char *name;
} net_device_t;

extern net_device_t g_net_dev;

#endif /* NET_H */