/**
 * @file PacketInfoIpv6Udp.hpp
 * @author Tobias
 * @brief class to provide packets IPv6 and UDP header information
 * @date 2021-06-09
 *
 */

#pragma once

#include <rte_byteorder.h>
#include <rte_ether.h>
#include <rte_icmp.h>
#include <rte_ip.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_tcp.h>
#include <rte_udp.h>

#include <stdexcept>

#include "PacketDissection/PacketInfo.hpp"

class PacketInfoIpv6Udp : public PacketInfo {
  public:
    inline PacketInfoIpv6Udp() { /*set_type(IPv6UDP);*/
    }

    inline void set_ip_hdr(rte_ipv6_hdr* ip6_hdr) { this->_ip6_hdr = ip6_hdr; }

    inline void set_udp_hdr(rte_udp_hdr* udp_hdr) { this->_udp_hdr = udp_hdr; }

    /// IPv6 Functions
    inline uint32_t get_dst_ip() {
        /*__uint128_t dest_ip = 0;
        for (short i = 0; i < 16; i++){
            dest_ip << 8;   /// shift left to make space for next part
            dest_ip = dest_ip + _ip6_hdr->dst_addr[i];   /// add next part
        }*/
        printf("PacketInfoIpv6Udp not yet implemented! ");
        return 0;
    }

    inline uint32_t get_src_ip() {
        /*__uint128_t src_ip = 0;
        for (short i = 0; i < 16; i++){
            src_ip << 8;   /// shift left to make space for next part
            src_ip = src_ip + _ip6_hdr->src_addr[i];   /// add next part
        }*/
        printf("PacketInfoIpv6Udp not yet implemented! ");
        return 0;
    }

    inline uint16_t get_dst_port() { return _udp_hdr->dst_port; }

    inline uint16_t get_src_port() { return _udp_hdr->src_port; }

    inline uint16_t get_packet_size() { return _ip6_hdr->payload_len + 320; }

    inline uint16_t get_payload_size() { return _ip6_hdr->payload_len - 8; }

  private:
    rte_ipv6_hdr* _ip6_hdr;
    rte_udp_hdr* _udp_hdr;
};