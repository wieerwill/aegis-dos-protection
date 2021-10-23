/**
 * @file PacketInfoIpv4Icmp.hpp
 * @author Tobias
 * @brief class to provide packets IPv4 and ICMP header information
 * @date 2021-06-08
 *
 */

#pragma once

#include <rte_byteorder.h>
#include <rte_icmp.h>

#include "PacketDissection/PacketInfoIpv4.hpp"

class PacketInfoIpv4Icmp : public PacketInfoIpv4 {
  public:
    inline PacketInfoIpv4Icmp();
    inline PacketInfoIpv4Icmp(rte_mbuf* mbuf, rte_ether_hdr* eth_hdr,
                              rte_ipv4_hdr* ip_hdr, rte_icmp_hdr* l4_hdr)
        : PacketInfoIpv4(IPv4ICMP, mbuf, eth_hdr, ip_hdr), _icmp_hdr(l4_hdr) {}
    inline ~PacketInfoIpv4Icmp() {
        //  PacketInfoIpv4::~PacketInfoIpv4();
        _icmp_hdr = nullptr;
    }

    /**
     * @brief Get packets payload size in byte
     *
     * @return uint16_t
     */
    inline uint16_t get_payload_size() { return 0; }

  private:
    rte_icmp_hdr* _icmp_hdr;

    /**
     * @brief Set the icmp header struct
     *
     * @param icmp_hdr
     */
    inline void set_icmp_hdr(rte_icmp_hdr* icmp_hdr) {
        this->_icmp_hdr = icmp_hdr;
    }
};