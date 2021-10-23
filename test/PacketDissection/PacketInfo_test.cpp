#include "ConfigurationManagement/Configurator.hpp"
#include "Initializer.hpp"
#include "PacketDissection/PacketInfo.hpp"
#include "PacketDissection/PacketInfoCreator.hpp"
#include "PacketDissection/PacketInfoIpv4Icmp.hpp"
#include "PacketDissection/PacketInfoIpv4Tcp.hpp"
#include "PacketDissection/PacketInfoIpv4Udp.hpp"
#include <catch2/catch.hpp>
#include <iostream>

TEST_CASE("Creation","[]")
{
    
    SECTION("PacketInfo","[]"){
      PacketInfo pkt_inf;
      rte_mbuf* mbuf;
      //REQUIRE_NOTHROW(pkt_inf.set_mbuf(mbuf));
      //REQUIRE_NOTHROW(nbuf = pkt_inf.get_mbuf());
      //CHECK(mbuf == nbuf);
      CHECK(pkt_inf.get_type() == NONE);
    }

    SECTION("Creation: IPv4ICMP", "[]") {
        // PacketInfoIpv4Icmp* pkt_inf =
        // static_cast<PacketInfoIpv4Icmp*>(PacketInfoCreator::create_pkt_info(IPv4ICMP));
        PacketInfoIpv4Icmp pkt_inf;
        CHECK(pkt_inf.get_type() == IPv4ICMP);
        // pkt_inf->set_mbuf(mbuf);
        // struct rte_ipv4_hdr* ip_hdr = rte_pktmbuf_mtod_offset(mbuf, struct
        // rte_ipv4_hdr*, 0); REQUIRE_NOTHROW(pkt_inf->set_ip_hdr(ip_hdr));
        // struct rte_icmp_hdr* l4_header = rte_pktmbuf_mtod_offset(mbuf, struct
        // rte_icmp_hdr*, 20);
        // REQUIRE_NOTHROW(pkt_inf->set_icmp_hdr(l4_header)); uint32_t num;
        // CHECK_NOTHROW(num= pkt_inf->get_dst_ip());
        // CHECK_NOTHROW(num= pkt_inf->get_src_ip());
        // CHECK_NOTHROW(num= pkt_inf->get_packet_size());
        // CHECK_NOTHROW(num= pkt_inf->get_payload_size());
    }

    SECTION("Creation: IPv4TCP", "[]") {
        // PacketInfoIpv4Tcp* pkt_inf =
        // static_cast<PacketInfoIpv4Tcp*>(PacketInfoCreator::create_pkt_info(IPv4TCP));
        PacketInfoIpv4Tcp pkt_inf;
        CHECK(pkt_inf.get_type() == IPv4TCP);
        // pkt_inf->set_mbuf(mbuf);
        // struct rte_ipv4_hdr* ip_hdr = rte_pktmbuf_mtod_offset(mbuf, struct
        // rte_ipv4_hdr*, 0); REQUIRE_NOTHROW(pkt_inf->set_ip_hdr(ip_hdr));
        // struct rte_tcp_hdr* l4_header = rte_pktmbuf_mtod_offset(mbuf, struct
        // rte_tcp_hdr*, 20); REQUIRE_NOTHROW(pkt_inf->set_tcp_hdr(l4_header));
        // uint32_t num;
        // uint32_t num2;
        // CHECK_NOTHROW(num= pkt_inf->get_dst_ip());
        // CHECK_NOTHROW(num= pkt_inf->get_src_ip());
        // CHECK_NOTHROW(num= pkt_inf->get_packet_size());
        // CHECK_NOTHROW(num= pkt_inf->get_payload_size());
        // CHECK_NOTHROW(num= pkt_inf->get_dst_port());
        // CHECK_NOTHROW(num= pkt_inf->get_src_port());
        // CHECK_NOTHROW(num= pkt_inf->get_flags());
        // CHECK_NOTHROW(num= pkt_inf->get_window_size());
        // CHECK_NOTHROW(pkt_inf->set_ack_num(num));
        // CHECK_NOTHROW(num2= pkt_inf->get_ack_num());
        // CHECK(num == num2);
        // CHECK_NOTHROW(pkt_inf->set_seq_num(num));
        // CHECK_NOTHROW(num = pkt_inf->get_seq_num());
        // CHECK(num == num2);
    }

    SECTION("Creation: IPv4UDP", "[]") {
        // PacketInfoIpv4Udp* pkt_inf =
        // static_cast<PacketInfoIpv4Udp*>(PacketInfoCreator::create_pkt_info(IPv4UDP));
        PacketInfoIpv4Udp pkt_inf;
        CHECK(pkt_inf.get_type() == IPv4UDP);
        // pkt_inf->set_mbuf(mbuf);
        // struct rte_ipv4_hdr* ip_hdr = rte_pktmbuf_mtod_offset(mbuf, struct
        // rte_ipv4_hdr*, 0); REQUIRE_NOTHROW(pkt_inf->set_ip_hdr(ip_hdr));
        // struct rte_udp_hdr* l4_header = rte_pktmbuf_mtod_offset(mbuf, struct
        // rte_udp_hdr*, 20); REQUIRE_NOTHROW(pkt_inf->set_udp_hdr(l4_header));
        // uint32_t num;
        // CHECK_NOTHROW(num= pkt_inf->get_dst_ip());
        // CHECK_NOTHROW(num= pkt_inf->get_src_ip());
        // CHECK_NOTHROW(num= pkt_inf->get_packet_size());
        // CHECK_NOTHROW(num= pkt_inf->get_payload_size());
        // CHECK_NOTHROW(num= pkt_inf->get_dst_port());
        // CHECK_NOTHROW(num= pkt_inf->get_src_port());
    }
}

TEST_CASE("Transformation", "[]") {

    SECTION("keeping Type", "[]") {
        PacketInfo* pkt_inf;
        // pkt_inf = PacketInfoCreator::create_pkt_info(IPv4ICMP);
        pkt_inf = new PacketInfoIpv4Icmp;
        PacketInfoIpv4Icmp* pkt_inf_icmp;
        pkt_inf_icmp = static_cast<PacketInfoIpv4Icmp*>(pkt_inf);
        CHECK(pkt_inf_icmp->get_type() == IPv4ICMP);

        // PacketInfoCreator::create_pkt_info(IPv4TCP)
        pkt_inf = new PacketInfoIpv4Tcp;
        PacketInfoIpv4Tcp* pkt_inf_tcp;
        pkt_inf_tcp = static_cast<PacketInfoIpv4Tcp*>(pkt_inf);
        CHECK(pkt_inf_tcp->get_type() == IPv4TCP);

        // PacketInfoCreator::create_pkt_info(IPv4UDP)
        pkt_inf = new PacketInfoIpv4Udp;
        PacketInfoIpv4Udp* pkt_inf_udp;
        pkt_inf_udp = static_cast<PacketInfoIpv4Udp*>(pkt_inf);
        CHECK(pkt_inf_udp->get_type() == IPv4UDP);

        // PacketInfoCreator::create_pkt_info(NONE)
        pkt_inf = new PacketInfo;
        CHECK(pkt_inf->get_type() == NONE);

        PacketInfo* pkt_inf_arr[5];
        pkt_inf_arr[0] = pkt_inf_icmp;
        pkt_inf_arr[1] = pkt_inf_tcp;
        pkt_inf_arr[2] = pkt_inf_udp;
        pkt_inf_arr[3] = pkt_inf;
        CHECK(pkt_inf_arr[0]->get_type() == IPv4ICMP);
        CHECK(pkt_inf_arr[1]->get_type() == IPv4TCP);
        CHECK(pkt_inf_arr[2]->get_type() == IPv4UDP);
        CHECK(pkt_inf_arr[3]->get_type() == NONE);

        // clean up
        delete pkt_inf;
        delete pkt_inf_icmp;
        delete pkt_inf_tcp;
        delete pkt_inf_udp;
        delete pkt_inf_arr;
    }
}

