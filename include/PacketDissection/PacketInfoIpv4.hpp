/**
 * @file PacketInfoIpv4.hpp
 * @author Tobias
 * @brief class which provides IPv4 header information
 *
 */

#pragma once

#include <boost/log/trivial.hpp>

#include <rte_byteorder.h>
#include <rte_ip.h>
#include <rte_mbuf.h>

#include "PacketDissection/PacketInfo.hpp"
#include "Definitions.hpp"

#define IP_HDR_LEN 20
#define VERSION_AND_IP_HDR_LEN 0b01000101

class PacketInfoIpv4 : public PacketInfo {
  public:
    PacketInfoIpv4();
    inline PacketInfoIpv4(rte_mbuf* const mbuf, rte_ether_hdr* const eth_hdr,
                          rte_ipv4_hdr* const ip_hdr)
        : PacketInfo(IPv4, mbuf, eth_hdr), _ip4_hdr(ip_hdr) {}
    inline ~PacketInfoIpv4() {
        // PacketInfo::~PacketInfo();
    }

    /**
     * @brief Get packets destination IP address
     *
     * @return uint32_t
     */
    inline uint32_t get_dst_ip() {
        return rte_be_to_cpu_32(_ip4_hdr->dst_addr);
    }

    /**
     * @brief Get packets source IP address
     *
     * @return uint32_t
     */
    inline uint32_t get_src_ip() {
        return rte_be_to_cpu_32(_ip4_hdr->src_addr);
    }

    /**
     * @brief Set packets source IP address
     *
     * @param ip
     */
    inline void set_src_ip(uint32_t ip) {
        _ip4_hdr->src_addr = rte_cpu_to_be_32(ip);
    }

    /**
     * @brief Get packets size in byte
     *
     * @return uint16_t
     */
    inline uint16_t get_packet_size() {
        return rte_be_to_cpu_16(_ip4_hdr->total_length);
    }

    /**
     * @brief Get the ip header struct
     *
     * @return rte_ipv4_hdr*
     */
    inline rte_ipv4_hdr* const get_ip_hdr() { return _ip4_hdr; }

    void set_ip_cksm(uint16_t cksm);

  protected:
    inline PacketInfoIpv4(PacketType const type, rte_mbuf* const mbuf,
                          rte_ether_hdr* const eth_hdr,
                          rte_ipv4_hdr* const ip_hdr)
        : PacketInfo(type, mbuf, eth_hdr), _ip4_hdr(ip_hdr) {}

    inline PacketInfoIpv4(PacketType const type, rte_mbuf* const mbuf,
                          rte_ipv4_hdr* const ip_hdr)
        : PacketInfo(type, mbuf), _ip4_hdr(ip_hdr) {}

    /**
     * @brief fills this packets IPv4 header with neseceties
     *
     * @param src_ip IP from which packet was originally send from
     * @param dst_ip IP packet is send to
     * @param proto protocol whose header follows after Ip-header
     * @param payload_len packets number of bytes without IP-header
     */
    inline void fill_ip_hdr(uint32_t src_ip, uint32_t dst_ip, uint8_t proto,
                            uint16_t payload_len) {

        _ip4_hdr->src_addr = rte_cpu_to_be_32(src_ip);
        _ip4_hdr->dst_addr = rte_cpu_to_be_32(dst_ip);

        _ip4_hdr->version_ihl = VERSION_AND_IP_HDR_LEN;
        _ip4_hdr->type_of_service = 0;
        _ip4_hdr->total_length = rte_be_to_cpu_16(payload_len + IP_HDR_LEN);
        _ip4_hdr->packet_id = 0;
        _ip4_hdr->fragment_offset = 0;
        _ip4_hdr->time_to_live = 128;
        _ip4_hdr->next_proto_id = proto;
        _ip4_hdr->hdr_checksum = 0;
    }

    inline void recalculate_ip_checksum() {
        _ip4_hdr->hdr_checksum = rte_ipv4_cksum(_ip4_hdr);
    }

    /**
     * @brief Get IP pseudo header checksum
     *
     * @return uint16_t IP pseudo header checksum
     */
    inline uint16_t get_pseudo_hdr_cksm() {
        rte_mbuf* mbuf = get_mbuf();
        mbuf->l2_len = sizeof(struct rte_ether_hdr);
        mbuf->l3_len = sizeof(struct rte_ipv4_hdr);
        return rte_ipv4_phdr_cksum(_ip4_hdr, mbuf->ol_flags);
    }

    inline uint8_t get_ip_hdr_len() {
        uint8_t len = _ip4_hdr->version_ihl & 0b00001111;
        len = len * 4;
        return len;
    }

  private:
    rte_ipv4_hdr* const _ip4_hdr;
};