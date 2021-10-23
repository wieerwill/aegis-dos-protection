/**
 * @file PacketProtUdp.hpp
 * @author Tobias
 * @brief provide UDP specific functions
 *
 */

#pragma once

#include <rte_byteorder.h>
#include <rte_ip.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_udp.h>

class PacketProtUdp {
  public:
    /**
     * @brief Get packets UDP destination port
     *
     * @return uint16_t
     */
    inline static uint16_t get_dst_port(rte_udp_hdr* const udp_hdr) {
        return rte_be_to_cpu_16(udp_hdr->dst_port);
    }

    /**
     * @brief Get packets UDP source port
     *
     * @return uint16_t
     */
    inline static uint16_t get_src_port(rte_udp_hdr* const udp_hdr) {
        return rte_be_to_cpu_16(udp_hdr->src_port);
    }


    inline static void fill_payloadless_udp_header(rte_udp_hdr* const udp_hdr,
        uint16_t dst_port, uint16_t src_port) {

        udp_hdr->dst_port = rte_cpu_to_be_16(dst_port);
        udp_hdr->src_port = rte_cpu_to_be_16(src_port);
        udp_hdr->dgram_len = rte_cpu_to_be_16(8);
        udp_hdr->dgram_cksum = 0;
    }
};