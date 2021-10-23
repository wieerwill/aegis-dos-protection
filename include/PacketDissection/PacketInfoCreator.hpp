/**
 * @file PacketInfoCreator.hpp
 * @author Tobias
 * @brief
 * @date 2021-06-16
 */
#pragma once

#include <rte_ether.h>

#include <boost/log/trivial.hpp>

#include "Definitions.hpp"
#include "PacketDissection/PacketInfo.hpp"
#include "PacketDissection/PacketInfoIpv4.hpp"
#include "PacketDissection/PacketInfoIpv4Icmp.hpp"
#include "PacketDissection/PacketInfoIpv4Tcp.hpp"
#include "PacketDissection/PacketInfoIpv4Udp.hpp"

#define RTE_ETHER_ADDR_LEN 6

#define ETHER_TYPE_IPv4 0x0800

class PacketInfoCreator {
  public:
    /**
     * @brief Create a PacketInfo object for given mbuf
     *
     * @param mbuf dpdk abstraction for one packet
     * @return PacketInfo*
     */
    inline static PacketInfo* create_pkt_info(rte_mbuf* mbuf) {
        struct rte_ether_hdr* eth_hdr;
        eth_hdr = rte_pktmbuf_mtod(mbuf, struct rte_ether_hdr*);
        uint16_t offset = (uint16_t)sizeof(struct rte_ether_hdr);

        /// from here on we know the l3 type
        if (rte_be_to_cpu_16(eth_hdr->ether_type) == ETHER_TYPE_IPv4) {
            struct rte_ipv4_hdr* ip4_hdr;
            ip4_hdr =
                rte_pktmbuf_mtod_offset(mbuf, struct rte_ipv4_hdr*, offset);
            uint8_t protocol_id = ip4_hdr->next_proto_id;

            if (protocol_id == 6) { // TCP
                // calculate point where TCP-header begins
                offset = offset + (ip4_hdr->version_ihl - 64) * 4;
                // map TCP-header on memory
                struct rte_tcp_hdr* tcp_hdr;
                tcp_hdr =
                    rte_pktmbuf_mtod_offset(mbuf, struct rte_tcp_hdr*, offset);
                // create PacketInfo
                PacketInfoIpv4Tcp* pkt_inf =
                    new PacketInfoIpv4Tcp(mbuf, eth_hdr, ip4_hdr, tcp_hdr);

                return pkt_inf;
            } else if (protocol_id == 17) { // UDP
                offset = offset + (ip4_hdr->version_ihl - 64) * 4;
                struct rte_udp_hdr* udp_hdr;
                udp_hdr =
                    rte_pktmbuf_mtod_offset(mbuf, struct rte_udp_hdr*, offset);

                PacketInfoIpv4Udp* pkt_inf =
                    new PacketInfoIpv4Udp(mbuf, eth_hdr, ip4_hdr, udp_hdr);

                return pkt_inf;
            } else if (protocol_id == 1) { // ICMP
                offset = offset + (ip4_hdr->version_ihl - 64) * 4;
                struct rte_icmp_hdr* icmp_hdr;
                icmp_hdr =
                    rte_pktmbuf_mtod_offset(mbuf, struct rte_icmp_hdr*, offset);
                PacketInfoIpv4Icmp* pkt_inf =
                    new PacketInfoIpv4Icmp(mbuf, eth_hdr, ip4_hdr, icmp_hdr);

                return pkt_inf;
            } else {
                printf("packet is neither TCP UDP nor ICMP");
                return new PacketInfo(mbuf, eth_hdr);
            }
        } else if (rte_be_to_cpu_16(eth_hdr->ether_type) == 0x86DD) {
            /// \TODO: implement IPv6
            return new PacketInfo(mbuf, eth_hdr);
        } else if (rte_be_to_cpu_16(eth_hdr->ether_type) == 0x0806) {
            /// \TODO: implement ARP
            return new PacketInfo(mbuf, eth_hdr);
        } else {
            return new PacketInfo(mbuf, eth_hdr);
        }
    }

    /**
     * @brief Create a PacketInfo object for given packet type
     *
     * @param mbuf dpdk abstraction for one packet
     * @param type specifies Type which PacketInfo should have
     * @return PacketInfo*
     */
    inline static PacketInfo* create_pkt_info(rte_mbuf* mbuf, PacketType type) {

        int off_set = sizeof(struct rte_ether_hdr);
        struct rte_ether_hdr* eth_hdr;
        rte_pktmbuf_append(mbuf, sizeof(struct rte_ether_hdr));
        eth_hdr = rte_pktmbuf_mtod(mbuf, struct rte_ether_hdr*);

        switch (type) {
        case IPv4ICMP: {
            eth_hdr->ether_type = rte_cpu_to_be_16(ETHER_TYPE_IPv4);

            struct rte_ipv4_hdr* ip4_hdr;
            rte_pktmbuf_append(mbuf, sizeof(rte_ipv4_hdr));
            ip4_hdr =
                rte_pktmbuf_mtod_offset(mbuf, struct rte_ipv4_hdr*, off_set);
            off_set = off_set + sizeof(rte_ipv4_hdr);

            rte_pktmbuf_append(mbuf, sizeof(rte_icmp_hdr));
            rte_icmp_hdr* icmp_hdr;
            icmp_hdr =
                rte_pktmbuf_mtod_offset(mbuf, struct rte_icmp_hdr*, off_set);

            return new PacketInfoIpv4Icmp(mbuf, eth_hdr, ip4_hdr, icmp_hdr);
        } break;

        case IPv4TCP: {
            eth_hdr->ether_type = rte_cpu_to_be_16(ETHER_TYPE_IPv4); // 0x0008

            struct rte_ipv4_hdr* ip4_hdr;
            rte_pktmbuf_append(mbuf,
                               sizeof(rte_ipv4_hdr) + sizeof(rte_tcp_hdr));
            ip4_hdr =
                rte_pktmbuf_mtod_offset(mbuf, struct rte_ipv4_hdr*, off_set);
            off_set = off_set + sizeof(rte_ipv4_hdr);

            rte_tcp_hdr* tcp_hdr;
            tcp_hdr =
                rte_pktmbuf_mtod_offset(mbuf, struct rte_tcp_hdr*, off_set);

            return new PacketInfoIpv4Tcp(mbuf, eth_hdr, ip4_hdr, tcp_hdr);
        } break;

        case IPv4UDP: {
            eth_hdr->ether_type = rte_cpu_to_be_16(ETHER_TYPE_IPv4);

            struct rte_ipv4_hdr* ip4_hdr;
            rte_pktmbuf_append(mbuf, sizeof(rte_ipv4_hdr));
            ip4_hdr =
                rte_pktmbuf_mtod_offset(mbuf, struct rte_ipv4_hdr*, off_set);
            off_set = off_set + sizeof(rte_ipv4_hdr);

            rte_pktmbuf_append(mbuf, sizeof(rte_udp_hdr));
            rte_udp_hdr* udp_hdr;
            udp_hdr =
                rte_pktmbuf_mtod_offset(mbuf, struct rte_udp_hdr*, off_set);

            return new PacketInfoIpv4Udp(mbuf, eth_hdr, ip4_hdr, udp_hdr);
        } break;

        case IPv6ICMP:
            return new PacketInfo(mbuf, eth_hdr);
            break;

        case IPv6TCP:
            return new PacketInfo(mbuf, eth_hdr);
            break;

        case IPv6UDP:
            return new PacketInfo(mbuf, eth_hdr);
            break;

        default:
            return new PacketInfo(mbuf, eth_hdr);
            break;
        }
    }

    /**
     * @brief Create a minimalized PacketInfo
     * this Packet is expected to already have a filled ether header
     * and is only for IPv4
     * @param mbuf dpdk abstraction for one packet
     * @param type specifies Type which PacketInfo should have
     * @return PacketInfo*
     */
    inline static PacketInfo* create_mini_pkt_info(rte_mbuf* mbuf,
                                                   PacketType type) {
        int off_set = sizeof(struct rte_ether_hdr);

        switch (type) {

        case IPv4TCP: {

            struct rte_ipv4_hdr* ip4_hdr;
            ip4_hdr =
                rte_pktmbuf_mtod_offset(mbuf, struct rte_ipv4_hdr*, off_set);
            off_set = off_set + sizeof(rte_ipv4_hdr);

            rte_tcp_hdr* tcp_hdr;
            tcp_hdr =
                rte_pktmbuf_mtod_offset(mbuf, struct rte_tcp_hdr*, off_set);

            return new PacketInfoIpv4Tcp(mbuf, ip4_hdr, tcp_hdr);
        } break;

        case IPv4UDP: {

            struct rte_ipv4_hdr* ip4_hdr;
            ip4_hdr =
                rte_pktmbuf_mtod_offset(mbuf, struct rte_ipv4_hdr*, off_set);
            off_set = off_set + sizeof(rte_ipv4_hdr);

            rte_udp_hdr* udp_hdr;
            udp_hdr =
                rte_pktmbuf_mtod_offset(mbuf, struct rte_udp_hdr*, off_set);

            return new PacketInfoIpv4Udp(mbuf, ip4_hdr, udp_hdr);
        } break;

        default:
            return nullptr;
            break;
        }
    }

    /**
     * @brief returns a packets destiantion ip
     * This method takes an mbuf and extract the IPv4 IP-adress.
     * In case the packet inside the mbuf is no IPv4 packet, 0 will be returned
     * @param mbuf structure to hand packets over
     * @return uint32_t packets IP-adress; it's 0, in case of no IPv4
     */
    inline static uint32_t get_dst_ip_from_mbuf(rte_mbuf* mbuf) {

        rte_ether_hdr* eth_hdr = rte_pktmbuf_mtod(mbuf, struct rte_ether_hdr*);

        if (eth_hdr->ether_type == rte_cpu_to_be_16(ETHER_TYPE_IPv4)) {

            int off_set = sizeof(struct rte_ether_hdr);
            rte_ipv4_hdr* ip_hdr =
                rte_pktmbuf_mtod_offset(mbuf, struct rte_ipv4_hdr*, off_set);
            return rte_be_to_cpu_32(ip_hdr->dst_addr);

        } else {
            return 0;
        }
    }

    inline static void destroy_pkt_info(PacketInfo* info) {

        PacketType type = info->get_type();
        switch (type) {

        case IPv4TCP: {
            delete static_cast<PacketInfoIpv4Tcp*>(info);
        } break;

        case IPv4UDP: {
            delete static_cast<PacketInfoIpv4Udp*>(info);
        } break;

        case NONE: {
            delete info;
        } break;
        case IPv4ICMP: {
            delete static_cast<PacketInfoIpv4Icmp*>(info);
        } break;

        default:
            break;
        }
    }

    inline static bool is_ipv4_tcp(rte_mbuf* mbuf) {
        struct rte_ether_hdr* eth_hdr;
        eth_hdr = rte_pktmbuf_mtod(mbuf, struct rte_ether_hdr*);

        if (rte_be_to_cpu_16(eth_hdr->ether_type) == ETHER_TYPE_IPv4) {
            uint16_t offset = (uint16_t)sizeof(struct rte_ether_hdr);
            struct rte_ipv4_hdr* ip4_hdr;
            ip4_hdr =
                rte_pktmbuf_mtod_offset(mbuf, struct rte_ipv4_hdr*, offset);
            uint8_t protocol_id = ip4_hdr->next_proto_id;

            if (protocol_id == 6) { // TCP
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
};