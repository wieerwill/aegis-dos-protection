#pragma once

#include <rte_byteorder.h>
#include <stdint.h>

struct rte_icmp_hdr {
    uint8_t icmp_type;      /* ICMP packet type. */
    uint8_t icmp_code;      /* ICMP packet code. */
    rte_be16_t icmp_cksum;  /* ICMP packet checksum. */
    rte_be16_t icmp_ident;  /* ICMP packet identifier. */
    rte_be16_t icmp_seq_nb; /* ICMP packet sequence number. */
} __attribute__((__packed__));