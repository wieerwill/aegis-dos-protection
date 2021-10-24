#pragma once

#include <rte_byteorder.h>
#include <rte_common.h>

#define RTE_ETHER_ADDR_LEN 6

struct rte_ether_addr {
    uint8_t addr_bytes[RTE_ETHER_ADDR_LEN];
} __attribute__((__packed__));

struct rte_ether_hdr {
    struct rte_ether_addr d_addr;
    RTE_STD_C11
    union {
        struct rte_ether_addr s_addr;
        struct {
            struct rte_ether_addr S_addr;
        } S_un;
    };
    rte_be16_t ether_type;
} __attribute__((__packed__));
