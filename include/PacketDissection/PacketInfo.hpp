/**
 * @file PacketInfo.hpp
 * @author Tobias
 * @brief class to provide general packet information
 * @date 2021-06-08
 *
 */

#pragma once

#include <rte_ether.h>
#include <rte_mbuf.h>

enum PacketType {
    NONE,
    IPv4ICMP,
    IPv4TCP,
    IPv4UDP,
    IPv6ICMP,
    IPv6TCP,
    IPv6UDP,
    IPv4,
    IPv6,
    ARP
};

class PacketInfo {
  public:
    inline PacketInfo() : _type(NONE), _mbuf(nullptr), _eth_hdr(nullptr) {}

    inline PacketInfo(rte_mbuf* const mbuf, rte_ether_hdr* const eth_hdr)
        : _type(NONE), _mbuf(mbuf), _eth_hdr(eth_hdr) {}

    inline ~PacketInfo() {
        //_mbuf = nullptr;
    }

    /**
     * @brief Get pointer to mbuf
     *
     * @return rte_mbuf*
     */
    inline rte_mbuf* const get_mbuf() { return _mbuf; }

    /**
     * @brief Get PacketInfos specialised type
     *
     * @return PacketType
     */
    inline PacketType const get_type() { return _type; }

    /**
     * @brief Get the source MAC-address
     *
     * @return rte_ether_addr
     */
    inline rte_ether_addr get_src_mac() { return _eth_hdr->s_addr; }

    /**
     * @brief Get the destination MAC-address
     *
     * @return rte_ether_addr
     */
    inline rte_ether_addr get_dst_mac() { return _eth_hdr->d_addr; }

  protected:
    inline PacketInfo(PacketType const type, rte_mbuf* const mbuf,
                      rte_ether_hdr* const eth_hdr)
        : _type(type), _mbuf(mbuf), _eth_hdr(eth_hdr) {}

    inline PacketInfo(PacketType const type, rte_mbuf* const mbuf)
        : _type(type), _mbuf(mbuf), _eth_hdr(nullptr) {}

    PacketType const _type;
    rte_ether_hdr* const _eth_hdr;

    inline rte_ether_hdr* const get_eth_hdr() { return _eth_hdr; }

    /**
     * @brief fill out standart ethernet header
     *
     * @param dst_mac MAC-address of destination
     * @param src_mac MAC-address of claimed source
     */
    inline void fill_eth_hdr(const rte_ether_addr& dst_mac,
                             const rte_ether_addr& src_mac) {
        _eth_hdr->d_addr = dst_mac;
        _eth_hdr->s_addr = src_mac;
    }

  private:
    rte_mbuf* const _mbuf;
};