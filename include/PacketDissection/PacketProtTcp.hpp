/**
 * @file PacketProtTcp.hpp
 * @author Tobias
 * @brief class to extract and change some informations in TCP header
 *
 */

#pragma once

#include <boost/log/trivial.hpp>
#include <rte_mbuf.h>
#include <rte_tcp.h>

#define FIN_FLAG_POS 0b00000001
#define SYN_FLAG_POS 0b00000010
#define RST_FLAG_POS 0b00000100
#define ACK_FLAG_POS 0b00010000
#define FIRST_4_BIT 0b11110000
#define TCP_HDR_SIZE_4BYTE_WORD 0b01010000

class PacketProtTcp {
  public:
    // PacketProtTcp(rte_tcp_hdr* tcp_hdr);

    /**
     * @brief Set packets TCP sequence number
     *
     * @param tcp_hdr
     * @param seq_num
     * \todo describe param tcp_hdr
     */
    inline static void set_seq_num(rte_tcp_hdr* tcp_hdr, uint32_t seq_num) {
        tcp_hdr->sent_seq = rte_cpu_to_be_32(seq_num);
    }

    /**
     * @brief Set packets TCP acknowledgment number
     *
     * @param tcp_hdr
     * @param ack_num
     * \todo describe param tcp_hdr
     */
    inline static void set_ack_num(rte_tcp_hdr* tcp_hdr, uint32_t ack_num) {
        tcp_hdr->recv_ack = rte_cpu_to_be_32(ack_num);
    }

    /**
     * @brief Get packets TCP destination port
     *
     * @param tcp_hdr
     * @return uint16_t
     *
     * \todo describe param tcp_hdr
     */
    inline static uint16_t get_dst_port(rte_tcp_hdr* tcp_hdr) {
        return rte_be_to_cpu_16(tcp_hdr->dst_port);
    }

    /**
     * @brief Get packets TCP source port
     *
     * @param tcp_hdr
     * @return uint16_t
     * \todo describe param tcp_hdr
     */
    inline static uint16_t get_src_port(rte_tcp_hdr* tcp_hdr) {
        return rte_be_to_cpu_16(tcp_hdr->src_port);
    }

    /**
     * @brief Get packets TCP flags
     * MSB is CWR flag, LSB is FIN flag, NS flag not included
     * @param tcp_hdr
     * @return uint8_t
     * \todo describe param tcp_hdr
     */
    inline static uint8_t get_flags(rte_tcp_hdr* tcp_hdr) {
        return tcp_hdr->tcp_flags; // these are FIN to CWR flag, but i am not
                                   // shure in which order
    }

    /**
     * @brief Get packets TCP window size
     *
     * @return uint16_t
     */
    inline static uint16_t get_window_size(rte_tcp_hdr* tcp_hdr) {
        return rte_be_to_cpu_16(tcp_hdr->rx_win);
    }

    /**
     * @brief Get packets TCP sequence number
     *
     * @return uint32_t
     */
    inline static uint32_t get_seq_num(rte_tcp_hdr* tcp_hdr) {
        return rte_be_to_cpu_32(tcp_hdr->sent_seq);
    }

    /**
     * @brief Get packets TCP acknowledgment number
     *
     * @return uint32_t
     */
    inline static uint32_t get_ack_num(rte_tcp_hdr* tcp_hdr) {
        return rte_be_to_cpu_32(tcp_hdr->recv_ack);
    }

    /**
     * @brief Get packets TCP-header length
     *
     * @return uint16_t
     */
    inline static uint16_t get_tcp_hdr_len(rte_tcp_hdr* tcp_hdr) {
        return (tcp_hdr->data_off & FIRST_4_BIT) * 4;
    }

    /**
     * @brief Get packets TCP SYN flag
     *
     * @return true if flag is set
     * @return false if flag is not set
     */
    inline static bool get_syn_flag(rte_tcp_hdr* tcp_hdr) {
        if ((tcp_hdr->tcp_flags & SYN_FLAG_POS) == SYN_FLAG_POS) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * @brief Get packets TCP ACK flag
     *
     * @return true if flag is set
     * @return false if flag is not set
     */
    inline static bool get_ack_flag(rte_tcp_hdr* tcp_hdr) {
        if ((tcp_hdr->tcp_flags & ACK_FLAG_POS) == ACK_FLAG_POS) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * @brief Get packets TCP RST flag
     *
     * @return true if flag is set
     * @return false if flag is not set
     */
    inline static bool get_rst_flag(rte_tcp_hdr* tcp_hdr) {
        if ((tcp_hdr->tcp_flags & RST_FLAG_POS) == RST_FLAG_POS) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * @brief Get packets TCP FIN flag
     *
     * @return true if flag is set
     * @return false if flag is not set
     */
    inline static bool get_fin_flag(rte_tcp_hdr* tcp_hdr) {
        if ((tcp_hdr->tcp_flags & FIN_FLAG_POS) == FIN_FLAG_POS) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * @brief fills empty mbuf with IP and TCP header
     * If this PacketInfo has a _mbuf and this _mbuf is empty,
     * then all IP and TCP header information is filled in.
     * This function doesn't create a new mbuf.
     * @param tcp_hdr
     * @param mbuf
     * @param src_ip IP address packet originally originated from
     * @param dst_ip IP address packet is going to be send to
     * @param src_port TCP port packet originally was send from
     * @param dst_port TCP port packet should be reci#include <rte_ip.h>eved on
     * @param seq_num TCP sequence number
     * @param ack_num TCP acknowledgment number
     * @param flags TCP flags wich are going to be set, can't set NS flag
     * @param rx_win TCP recive side window
     */

    inline static void
    fill_payloadless_tcp_header(rte_tcp_hdr* tcp_hdr, rte_mbuf* mbuf,
                                uint16_t src_port, uint16_t dst_port,
                                uint32_t seq_num, uint32_t ack_num,
                                uint8_t flags, uint16_t rx_win) {

        tcp_hdr->src_port = rte_cpu_to_be_16(src_port);
        tcp_hdr->dst_port = rte_cpu_to_be_16(dst_port);
        tcp_hdr->sent_seq = rte_cpu_to_be_32(seq_num);
        tcp_hdr->recv_ack = rte_cpu_to_be_32(ack_num);
        tcp_hdr->data_off = TCP_HDR_SIZE_4BYTE_WORD;
        tcp_hdr->tcp_flags = flags;
        tcp_hdr->rx_win = rte_cpu_to_be_16(rx_win);
        tcp_hdr->tcp_urp = 0;
        tcp_hdr->cksum = 0;
    }

    inline static void fill_tcp_cksm(rte_tcp_hdr* tcp_hdr, uint16_t cksm) {
        tcp_hdr->cksum = rte_cpu_to_be_16(cksm);
    }
};