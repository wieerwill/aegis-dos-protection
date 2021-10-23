#pragma once

#include <rte_byteorder.h>

struct rte_udp_hdr {
    rte_be16_t src_port;
    rte_be16_t dst_port;
    rte_be16_t dgram_len;
    rte_be16_t dgram_cksum;
} __attribute__((__packed__));