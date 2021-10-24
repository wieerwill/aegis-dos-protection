/**
 * @file PacketInfoIpv4Udp.hpp
 * @author Tobias
 * @brief class to provide packets IPv4 and UDP header information
 * @date 2021-06-08
 *
 */

#pragma once

#include <rte_byteorder.h>
#include <rte_udp.h>

#include "PacketDissection/PacketInfoIpv4.hpp"
#include "PacketDissection/PacketProtUdp.hpp"

#define UDP_HDR_LEN 8

class PacketInfoIpv4Udp : public PacketInfoIpv4 {
  public:
    inline PacketInfoIpv4Udp();

    inline PacketInfoIpv4Udp(rte_mbuf* const mbuf, rte_ether_hdr* const eth_hdr,
                             rte_ipv4_hdr* const ip_hdr,
                             rte_udp_hdr* const l4_hdr)
        : PacketInfoIpv4(IPv4UDP, mbuf, eth_hdr, ip_hdr), _udp_hdr(l4_hdr) {}

    inline PacketInfoIpv4Udp(rte_mbuf* const mbuf, rte_ipv4_hdr* const ip_hdr,
                             rte_udp_hdr* const l4_hdr)
        : PacketInfoIpv4(IPv4UDP, mbuf, ip_hdr), _udp_hdr(l4_hdr) {}

    // inline PacketInfoIpv4Udp::~PacketInfoIpv4Udp() {
    //    PacketInfoIpv4::~PacketInfoIpv4();
    //    //_udp_hdr = nullptr;
    //}

    /**
     * @brief Get packets UDP destination port
     *
     * @return uint16_t
     */
    inline uint16_t get_dst_port() {
        return PacketProtUdp::get_dst_port(_udp_hdr);
    }

    /**
     * @brief Get packets UDP source port
     *
     * @return uint16_t
     */
    inline uint16_t get_src_port() {
        return PacketProtUdp::get_src_port(_udp_hdr);
    }

    /**
     * @brief Get packets payload size in byte
     *
     * @return uint16_t
     */
    inline uint16_t get_payload_size() {
        uint16_t len = get_packet_size();
        len = len - get_ip_hdr_len();
        return len - UDP_HDR_LEN;
    }

    /**
     * @brief fills empty mbuf with IP and UDP heder
     * This PacketInfos _mbuf is going to be filled with all
     * IP and UDP header information, execpt checksums.
     * This function doesn't create a new mbuf.
     * @param src_mac MAC address packet was send from
     * @param dst_mac MAC address packet is going to be send to
     * @param src_ip IP address packet originally was send from
     * @param dst_ip IP address packet is going to be send to
     * @param src_port TCP port packet originally was send from
     * @param dst_port TCP port packet is going to be send to
     */
    inline void fill_payloadless_udp_packet(rte_ether_addr src_mac,
                                            rte_ether_addr dst_mac,
                                            uint32_t src_ip, uint32_t dst_ip,
                                            uint16_t src_port,
                                            uint16_t dst_port) {

        // let PacketInfo handle ethernet filling
        fill_eth_hdr(dst_mac, src_mac);

        // let prot objekt handle tcp filling
        PacketProtUdp::fill_payloadless_udp_header(_udp_hdr, src_port,
                                                   dst_port);

        // let PacketInfoIpv4 handle IPv4 filling
        fill_ip_hdr(src_ip, dst_ip, 17, 8);
    }

    /**
     * @brief calculate current checksums and fill them in
     * Note, udp checksum doesn't have to be set in IPv4.
     * @param use_udp_cksm if true the UDP checksum is calculated
     */
    inline void recalculate_checksums(bool use_udp_cksm) {

        _udp_hdr->dgram_cksum = 0;

        if (use_udp_cksm == true) {

            rte_ipv4_hdr* ip_hdr = get_ip_hdr();
            ip_hdr->hdr_checksum = 0;
            _udp_hdr->dgram_cksum = rte_ipv4_udptcp_cksum(ip_hdr, _udp_hdr);
        }
        PacketInfoIpv4::recalculate_ip_checksum();
    }

    /**
     * @brief prepare mbuf for checksum calculation by hardware
     * Note, udp checksum doesn't have to be set in IPv4.
     * @param use_udp_cksm if true the UDP checksum will be calculated
     */
    inline void prepare_offloading_checksums(bool use_udp_cksm) {

        rte_mbuf* mbuf = get_mbuf();
        mbuf->ol_flags = PKT_TX_IPV4 | PKT_TX_IP_CKSUM;
        mbuf->l4_len = sizeof(struct rte_udp_hdr);

        if (use_udp_cksm == true) {
            mbuf->ol_flags |= PKT_TX_UDP_CKSUM;
            _udp_hdr->dgram_cksum = PacketInfoIpv4::get_pseudo_hdr_cksm();
        } else {
            mbuf->l2_len = sizeof(struct rte_ether_hdr);
            mbuf->l3_len = sizeof(struct rte_ipv4_hdr);
        }
    }

  private:
    rte_udp_hdr* const _udp_hdr;
};