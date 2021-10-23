/**
 * @file PacketInfoIpv6Tcp.hpp
 * @author Tobias
 * @brief class to provide packets IPv6 and TCP header information
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
#include <rte_tcp.h>
#include <rte_udp.h>

#include "PacketDissection/PacketInfo.hpp"

class PacketInfoIpv6Tcp : public PacketInfo {
  public:
    inline PacketInfoIpv6Tcp() { /*set_type(IPv6TCP);*/
    }

    inline void create_reply(rte_mbuf* answ_mbuf, uint32_t seq_num,
                             uint32_t ack_num);

    inline void tear_down_connection(rte_mbuf* return_mbuf,
                                     rte_mbuf* forward_mbuf,
                                     uint32_t forward_seq_num,
                                     uint32_t forward_ack_num);

    inline void set_ip_hdr(rte_ipv6_hdr* ip6_hdr) { this->_ip6_hdr = ip6_hdr; }

    inline void set_tcp_hdr(rte_tcp_hdr* tcp_hdr) { this->_tcp_hdr = tcp_hdr; }

    inline void set_seq_num(uint32_t seq_num) { _tcp_hdr->sent_seq = seq_num; }

    inline void set_ack_num(uint32_t ack_num) { _tcp_hdr->recv_ack = ack_num; }

    /// IPv6 Functions
    inline uint32_t get_dst_ip() {
        /*__uint128_t dest_ip = 0;
        for (short i = 0; i < 16; i++){
            dest_ip = dest_ip * 256;   /// shift left to make space for next
        part dest_ip = dest_ip + _ip6_hdr->dst_addr[i];   /// add next part
        }*/
        std::cout << "PacketInfoIpv6Tcp not yet implemented! ";
        return 0;
    }

    inline uint32_t get_src_ip() {
        /*__uint128_t src_ip = 0;
        for (short i = 0; i < 16; i++){
            src_ip = src_ip * 256;   /// shift left to make space for next part
            src_ip = src_ip + _ip6_hdr->src_addr[i];   /// add next part
        }*/
        printf("PacketInfoIpv6Tcp not yet implemented! ");
        return 0;
    }

    inline uint16_t get_dst_port() { return _tcp_hdr->dst_port; }

    inline uint16_t get_src_port() { return _tcp_hdr->src_port; }

    inline uint8_t get_flags() { return _tcp_hdr->tcp_flags; }

    inline uint16_t get_packet_size() { return _ip6_hdr->payload_len + 320; }

    inline uint16_t get_payload_size() { return _ip6_hdr->payload_len - 20; }

    inline uint16_t get_window_size() { return _tcp_hdr->rx_win; }

    inline uint32_t get_seq_num() { return _tcp_hdr->sent_seq; }

    inline uint32_t get_ack_num() { return _tcp_hdr->recv_ack; }

    /**
     * @brief NOT YET IMPLEMENTED FOR IPv6
     * fills empty mbuf with IP and TCP header
     * If this PacketInfo has a _mbuf and this _mbuf is empty,
     * then all IP and TCP header information is filled in.
     * This function doesn't create a new mbuf.
     * @param src_ip IP address packet originally originated from
     * @param dst_ip IP address packet is going to be send to
     * @param src_port TCP port packet originally was send from
     * @param dst_port TCP port packet should be recieved on
     * @param seq_num TCP sequence number
     * @param ack_num TCP acknowledgment number
     * @param flags TCP flags wich are going to be set, can't set NS flag
     */
    inline void fill_payloadless_tcp_packet(uint32_t src_ip, uint32_t dst_ip,
                                            uint16_t src_port,
                                            uint16_t dst_port, uint32_t seq_num,
                                            uint32_t ack_num, uint8_t flags);

    inline void re_calculate_checksums(); ///> NOT YET IMPLEMENTED FOR IPv6

  private:
    rte_ipv6_hdr* _ip6_hdr;
    rte_tcp_hdr* _tcp_hdr;
};