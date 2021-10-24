#pragma once

#include <rte_byteorder.h>
#include <stdint.h>

struct rte_tcp_hdr {
    rte_be16_t src_port;
    rte_be16_t dst_port;
    rte_be32_t sent_seq;
    rte_be32_t recv_ack;
    uint8_t data_off;
    uint8_t tcp_flags;
    rte_be16_t rx_win;
    rte_be16_t cksum;
    rte_be16_t tcp_urp;
} __attribute__((__packed__));