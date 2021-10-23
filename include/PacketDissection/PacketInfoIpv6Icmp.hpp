/**
 * @file PacketInfoIpv6Icmp.hpp
 * @author Tobias
 * @brief class to provide packets IPv6 and ICMP header information
 * @date 2021-06-09
 *
 */

#pragma once

#include <iostream>

#include <rte_byteorder.h>
#include <rte_ether.h>
#include <rte_icmp.h>
#include <rte_ip.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#include "PacketDissection/PacketInfo.hpp"

class PacketInfoIpv6Icmp : public PacketInfo {
  public:
    inline PacketInfoIpv6Icmp() { /*set_type(IPv6ICMP);*/
    }
    inline void set_ip_hdr(rte_ipv6_hdr* ip6_hdr) { this->_ip6_hdr = ip6_hdr; }
    inline void set_icmp_hdr(rte_icmp_hdr* icmp_hdr) {
        this->_icmp_hdr = icmp_hdr;
    }

    /// IPv6 Functions
    inline uint32_t get_dst_ip() {
        /*__uint128_t dest_ip = 0;
        for (short i = 0; i < 16; i++){
            dest_ip << 8;   /// shift left to make space for next part
            dest_ip = dest_ip + _ip6_hdr->dst_addr[i];   /// add next part
        }*/
        std::cout << "PacketInfoIpv6Icmp not yet implemented! ";
        return 0;
    }

    inline uint32_t get_src_ip() {
        /*__uint128_t src_ip = 0;
        for (short i = 0; i < 16; i++){
            src_ip << 8;   /// shift left to make space for next part
            src_ip = src_ip + _ip6_hdr->src_addr[i];   /// add next part
        }*/
        printf("PacketInfoIpv6Icmp not yet implemented! ");
        return 0;
    }

    inline uint16_t get_packet_size() { return 384; }
    inline uint16_t get_payload_size() { return 0; }

  private:
    rte_ipv6_hdr* _ip6_hdr;
    rte_icmp_hdr* _icmp_hdr;
};