/**
 * @file PacketInfoIpv4Tcp.hpp
 * @author Tobias
 * @brief class to provide packets IPv4 and TCP header information
 * @date 2021-06-08
 *
 */

#pragma once

#include <iostream>

#include <rte_byteorder.h>
#include <rte_tcp.h>

#include "PacketDissection/PacketInfoIpv4.hpp"
#include "PacketDissection/PacketProtTcp.hpp"
#include "DebugHelper.hpp"
#include "Definitions.hpp"

class PacketInfoIpv4Tcp : public PacketInfoIpv4 {
  public:
    inline PacketInfoIpv4Tcp();

    inline PacketInfoIpv4Tcp(rte_mbuf* mbuf, rte_ether_hdr* eth_hdr,
                             rte_ipv4_hdr* ip_hdr, rte_tcp_hdr* l4_hdr)
        : PacketInfoIpv4(IPv4TCP, mbuf, eth_hdr, ip_hdr), _tcp_hdr(l4_hdr) {}

    inline PacketInfoIpv4Tcp(rte_mbuf* mbuf, rte_ipv4_hdr* ip_hdr,
                             rte_tcp_hdr* l4_hdr)
        : PacketInfoIpv4(IPv4TCP, mbuf, ip_hdr), _tcp_hdr(l4_hdr) {}

    inline ~PacketInfoIpv4Tcp() {
        //   PacketInfoIpv4::~PacketInfoIpv4();
        _tcp_hdr = nullptr;
    }

    /**
     * @brief Set packets TCP sequence number
     *
     * @param seq_num
     */
    inline void set_seq_num(uint32_t seq_num) {
        PacketProtTcp::set_seq_num(_tcp_hdr, seq_num);
    }

    /**
     * @brief Set packets TCP acknowledgment number
     *
     * @param ack_num
     */
    inline void set_ack_num(uint32_t ack_num) {
        PacketProtTcp::set_ack_num(_tcp_hdr, ack_num);
    }

    /**
     * @brief Get packets TCP destination port
     *
     * @return uint16_t
     */
    inline uint16_t get_dst_port() {
        return PacketProtTcp::get_dst_port(_tcp_hdr);
    }

    /**
     * @brief Get packets TCP source port
     *
     * @return uint16_t
     */
    inline uint16_t get_src_port() {
        return PacketProtTcp::get_src_port(_tcp_hdr);
    }

    /**
     * @brief Get packets TCP flags
     * MSB is CWR flag, LSB is FIN flag, NS flag not included
     * @return uint8_t
     */
    inline uint8_t get_flags() {
        //const char desc[] = "mbuf";
        return PacketProtTcp::get_flags(
            _tcp_hdr); // these are FIN to CWR flag, but
                       // i am not shure in which order
    }

    /**
     * @brief Get packets payload size in byte
     *
     * @return uint16_t
     */
    inline uint16_t get_payload_size() {
        uint16_t length = get_packet_size();
        length = length - get_ip_hdr_len();
        length = length - PacketProtTcp::get_tcp_hdr_len(_tcp_hdr);
        return length;
    }

    /**
     * @brief Get packets TCP window size
     *
     * @return uint16_t
     */
    inline uint16_t get_window_size() {
        return PacketProtTcp::get_window_size(_tcp_hdr);
    }

    /**
     * @brief Get packets TCP sequence number
     *
     * @return uint32_t
     */
    inline uint32_t get_seq_num() {
        return PacketProtTcp::get_seq_num(_tcp_hdr);
    }

    /**
     * @brief Get packets TCP acknowledgment number
     *
     * @return uint32_t
     */
    inline uint32_t get_ack_num() {
        return PacketProtTcp::get_ack_num(_tcp_hdr);
    }

    /**
     * @brief Get packets TCP SYN flag
     *
     * @return true if flag is set
     * @return false if flag is not set
     */
    inline bool get_syn_flag() { return PacketProtTcp::get_syn_flag(_tcp_hdr); }

    /**
     * @brief Get packets TCP ACK flag
     *
     * @return true if flag is set
     * @return false if flag is not set
     */
    inline bool get_ack_flag() { return PacketProtTcp::get_ack_flag(_tcp_hdr); }

    /**
     * @brief Get packets TCP RST flag
     *
     * @return true if flag is set
     * @return false if flag is not set
     */
    inline bool get_rst_flag() { return PacketProtTcp::get_rst_flag(_tcp_hdr); }

    /**
     * @brief Get packets TCP FIN flag
     *
     * @return true if flag is set
     * @return false if flag is not set
     */
    inline bool get_fin_flag() { return PacketProtTcp::get_fin_flag(_tcp_hdr); }

    /**
     * @brief fills empty mbuf with IP and TCP header
     * If this PacketInfo has a _mbuf and this _mbuf is empty,
     * then all IP and TCP header information is filled in.
     * This function doesn't create a new mbuf.
     * @param src_mac MAC address packet was send from
     * @param dst_mac MAC address packet is going to be send to
     * @param src_ip IP address packet originally originated from
     * @param dst_ip IP address packet is going to be send to
     * @param src_port TCP port packet originally was send from
     * @param dst_port TCP port packet should be recieved on
     * @param seq_num TCP sequence number
     * @param ack_num TCP acknowledgment number
     * @param flags TCP flags wich are going to be set, can't set NS flag
     * @param tcp_window TCP recive window size
     */
    inline void fill_payloadless_tcp_packet(
        rte_ether_addr src_mac, rte_ether_addr dst_mac, uint32_t src_ip,
        uint32_t dst_ip, uint16_t src_port, uint16_t dst_port, uint32_t seq_num,
        uint32_t ack_num, uint8_t flags, uint16_t tcp_window) {

        // const char desc[] = "mbuf";

        // let PacketInfo handle ethernet filling
        fill_eth_hdr(dst_mac, src_mac);

        // let prot objekt handle tcp filling
        PacketProtTcp::fill_payloadless_tcp_header(_tcp_hdr, get_mbuf(),
                                                   src_port, dst_port, seq_num,
                                                   ack_num, flags, tcp_window);

        // let PacketInfoIpv4 handle IPv4 filling
        fill_ip_hdr(src_ip, dst_ip, 6, 20);
    }

    /**
     * @brief fills in current checksums
     * calculates current IPv4 and TCP checksum and changes
     * them inside the packet
     */
    inline void recalculate_checksums() {
        rte_ipv4_hdr* ip4_hdr = get_ip_hdr();

        _tcp_hdr->cksum = 0;
        ip4_hdr->hdr_checksum = 0;
        _tcp_hdr->cksum = rte_ipv4_udptcp_cksum(ip4_hdr, _tcp_hdr);
        PacketInfoIpv4::recalculate_ip_checksum();
    }

    inline void prepare_offloading_checksums() {

        rte_mbuf* mbuf = get_mbuf();
        mbuf->ol_flags = PKT_TX_IPV4 | PKT_TX_IP_CKSUM | PKT_TX_TCP_CKSUM;
        mbuf->l4_len = sizeof(struct rte_tcp_hdr);

        uint16_t cksm = PacketInfoIpv4::get_pseudo_hdr_cksm();
        PacketProtTcp::fill_tcp_cksm(_tcp_hdr, cksm);
    }

    inline void set_tcp_cksm(uint16_t cksm) {
        PacketProtTcp::fill_tcp_cksm(_tcp_hdr, cksm);
    }

  private:
    rte_tcp_hdr* _tcp_hdr;

};