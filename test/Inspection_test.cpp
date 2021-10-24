#include "Inspection.hpp"

#include "ConfigurationManagement/Configurator.hpp"
#include "PacketDissection/PacketContainer.hpp"
#include "PacketDissection/PacketInfo.hpp"
#include "PacketDissection/PacketInfoCreator.hpp"
#include "PacketDissection/PacketInfoIpv4Icmp.hpp"
#include "PacketDissection/PacketInfoIpv4Tcp.hpp"
#include "PacketDissection/PacketInfoIpv4Udp.hpp"
#include "Threads/AttackThread.h"

#include <catch2/catch.hpp>
#include <iostream>

TEST_CASE("init Inspection", "[]") {
    Inspection testInspection;
    REQUIRE_NOTHROW(testInspection.update_stats(0, 0, 0, 0, 0, 0, 0, 0, 0));
}

// Für folgende wird eine funktionierende Packet Dissection benötigt
TEST_CASE("check attack detection", "[]") {
    // setup config
    Configurator::instance()->read_config("../test/Inspection_config.json");
    // create packet container
    uint16_t inside_port = 0;
    uint16_t outside_port = 1;
    struct rte_mempool mbuf_pool_struct;
    struct rte_mempool* mbuf_pool = &mbuf_pool_struct;
    CHECK(mbuf_pool != nullptr);

    NetworkPacketHandler* pkt_handler = new NetworkPacketHandler(0, 0);
    CHECK(pkt_handler != nullptr);

    PacketContainer* pkt_container =
        new PacketContainer(pkt_handler, mbuf_pool, inside_port, outside_port);
    CHECK(pkt_container != nullptr);
    // inspection class
    Inspection testInspection;

    /// test SYN-FIN attack
    SECTION("SYN-FIN Attack", "[]") {
        PacketInfo* pkt_info = nullptr;
        for (int i = 0; i < 5; ++i) {
            pkt_info = pkt_container->get_empty_packet(IPv4TCP);
            PacketInfoIpv4Tcp* pkt_info =
                static_cast<PacketInfoIpv4Tcp*>(pkt_info);
            // create packet with SYN-FIN Flag into packet container
            pkt_info->fill_payloadless_tcp_packet(
                {00, 00, 00, 00, 00, 00}, {00, 00, 00, 00, 00, 00}, 0, 0, 0, 0,
                0, 0, 0b00000011, 100);
        }
        // packet container to inspection
        testInspection.analyze_container(pkt_container);
        // Check if packetcontainer empty
        CHECK(pkt_container->get_total_number_of_packets() == 0);
    }

    /// test SYN-FIN-ACK attack
    SECTION("SYN-FIN-ACK Attack", "[]") {
        PacketInfo* pkt_info = nullptr;
        for (int i = 0; i < 5; ++i) {
            pkt_info = pkt_container->get_empty_packet(IPv4TCP);
            PacketInfoIpv4Tcp* pkt_info =
                static_cast<PacketInfoIpv4Tcp*>(pkt_info);
            // create packet with SYN-FIN-ACK Flag into packet container
            pkt_info->fill_payloadless_tcp_packet(
                {00, 00, 00, 00, 00, 00}, {00, 00, 00, 00, 00, 00}, 0, 0, 0, 0,
                0, 0, 0b00010011, 100);
        }
        // packet container to inspection
        testInspection.analyze_container(pkt_container);
        // Check if packetcontainer empty
        CHECK(pkt_container->get_total_number_of_packets() == 0);
    }

    /// test Zero Window attack
    SECTION("Zero Window Attack", "[]") {
        PacketInfo* pkt_info = nullptr;
        for (int i = 0; i < 5; ++i) {
            pkt_info = pkt_container->get_empty_packet(IPv4TCP);
            PacketInfoIpv4Tcp* pkt_info =
                static_cast<PacketInfoIpv4Tcp*>(pkt_info);
            // create packet with 0 window into packet container
            pkt_info->fill_payloadless_tcp_packet({00, 00, 00, 00, 00, 00},
                                                  {00, 00, 00, 00, 00, 00}, 0,
                                                  0, 0, 0, 0, 0, 0, 0);
        }
        // packet container to inspection
        testInspection.analyze_container(pkt_container);
        // Check if packetcontainer empty
        CHECK(pkt_container->get_total_number_of_packets() == 0);
    }

    ///  test Small Window attack
    SECTION("Small Window Attack", "[]") {
        PacketInfo* pkt_info = nullptr;
        for (int i = 0; i < 5; ++i) {
            pkt_info = pkt_container->get_empty_packet(IPv4TCP);
            PacketInfoIpv4Tcp* pkt_info =
                static_cast<PacketInfoIpv4Tcp*>(pkt_info);
            // create packet with small Windows into packet container
            pkt_info->fill_payloadless_tcp_packet({00, 00, 00, 00, 00, 00},
                                                  {00, 00, 00, 00, 00, 00}, 0,
                                                  0, 0, 0, 0, 0, 0, i);
        }
        // packet container to inspection
        testInspection.analyze_container(pkt_container);
        // Check if packetcontainer empty
        CHECK(pkt_container->get_total_number_of_packets() == 0);
    }

    /// test UDP Flood attack
    SECTION("UDP Flood Attack", "[]") {
        PacketInfo* pkt_info = nullptr;
        // create udp packets into packet container
        for (int i = 0; i < 25; ++i) {
            pkt_info = pkt_container->get_empty_packet(IPv4UDP);
            PacketInfoIpv4Udp* pkt_info =
                static_cast<PacketInfoIpv4Udp*>(pkt_info);
        }
        // packet container to inspection
        testInspection.analyze_container(pkt_container);
        // Check if packetcontainer only has threshold packets left
        CHECK(pkt_container->get_total_number_of_packets() == 5);
    }

    /// test TCP Flood attack
    SECTION("TCP Flood Attack", "[]") {
        PacketInfo* pkt_info = nullptr;
        // create tcp packets into packet container
        for (int i = 0; i < 25; ++i) {
            pkt_info = pkt_container->get_empty_packet(IPv4TCP);
            PacketInfoIpv4Tcp* pkt_info =
                static_cast<PacketInfoIpv4Tcp*>(pkt_info);
            pkt_info->fill_payloadless_tcp_packet({00, 00, 00, 00, 00, 00},
                                                  {00, 00, 00, 00, 00, 00}, 0,
                                                  0, 0, 0, i, 0, 0, 100);
        }
        // packet container to inspection
        testInspection.analyze_container(pkt_container);
        // Check if packetcontainer only has threshold packets left
        CHECK(pkt_container->get_total_number_of_packets() == 5);
    }

    /// \todo test ICMP Flood attack
    SECTION("ICMP Flood Attack", "[]") {
        PacketInfo* pkt_info = nullptr;
        // create icmp packets into packet container
        for (int i = 0; i < 25; ++i) {
            pkt_info = pkt_container->get_empty_packet(IPv4ICMP);
            PacketInfoIpv4Icmp* pkt_info =
                static_cast<PacketInfoIpv4Icmp*>(pkt_info);
        }
        // packet container to inspection
        testInspection.analyze_container(pkt_container);
        // Check if packetcontainer only has threshold packets left
        CHECK(pkt_container->get_total_number_of_packets() == 5);
    }
}

TEST_CASE("check update function", "[]") {
    Inspection testInspection;

    SECTION("Null", "[]") {
        // update statistic with given numbers
        REQUIRE_NOTHROW(testInspection.update_stats(0, 0, 0, 0, 0, 0, 0, 0, 0));
        // check correct formulas
        CHECK(testInspection.get_UDP_packet_rate() == 0);  //< udp_pkt/duration
        CHECK(testInspection.get_TCP_packet_rate() == 0);  //< tcp_pkt/duration
        CHECK(testInspection.get_ICMP_packet_rate() == 0); //< icmp_pkt/duration
        CHECK(testInspection.get_attack_level() == 0);     //< no attacks
        CHECK(testInspection.get_UDP_threshold() == 5);    //< _threshold_UDP
        CHECK(testInspection.get_TCP_threshold() == 5);    //< _threshold_TCP
        CHECK(testInspection.get_ICMP_threshold() == 5);   //< _threshold_ICMP
    }

    SECTION("UDP rate", "[]") {
        REQUIRE_NOTHROW(
            testInspection.update_stats(10, 0, 0, 0, 0, 0, 0, 0, 1));
        CHECK(testInspection.get_UDP_packet_rate() == 10);
    }
    SECTION("TCP rate", "[]") {
        REQUIRE_NOTHROW(
            testInspection.update_stats(0, 10, 0, 0, 0, 0, 0, 0, 1));
        CHECK(testInspection.get_TCP_packet_rate() == 10);
    }
    SECTION("ICMP rate", "[]") {
        REQUIRE_NOTHROW(
            testInspection.update_stats(0, 0, 0, 10, 0, 0, 0, 0, 1));
        CHECK(testInspection.get_ICMP_packet_rate() == 10);
    }
    SECTION("UDP Flood", "[]") {
        REQUIRE_NOTHROW(
            testInspection.update_stats(10, 0, 0, 5, 0, 0, 0, 0, 1));
        CHECK(testInspection.get_UDP_packet_rate() == 10);
        CHECK(testInspection.get_attack_level() ==
              5); //< UDP_Floods * _UDP_flood_weight
        CHECK(testInspection.get_UDP_threshold() == 0); //< 5-1/5*5*5
    }
    SECTION("TCP Flood", "[]") {
        REQUIRE_NOTHROW(
            testInspection.update_stats(0, 10, 0, 0, 5, 0, 0, 0, 1));
        CHECK(testInspection.get_TCP_packet_rate() == 10);
        CHECK(testInspection.get_attack_level() == 5);
        CHECK(testInspection.get_TCP_threshold() == 0);
    }
    SECTION("ICMP Flood", "[]") {
        REQUIRE_NOTHROW(
            testInspection.update_stats(0, 0, 10, 0, 0, 5, 0, 0, 1));
        CHECK(testInspection.get_ICMP_packet_rate() == 10);
        CHECK(testInspection.get_attack_level() == 5);
        CHECK(testInspection.get_ICMP_threshold() == 0);
    }
    SECTION("SYN-FIN Attack", "[]") {
        REQUIRE_NOTHROW(
            testInspection.update_stats(0, 10, 0, 0, 0, 0, 5, 0, 1));
        CHECK(testInspection.get_TCP_packet_rate() == 10);
        CHECK(testInspection.get_attack_level() == 5);
        CHECK(testInspection.get_TCP_threshold() == 0);
    }
    SECTION("SmallWindow Attack", "[]") {
        REQUIRE_NOTHROW(
            testInspection.update_stats(0, 10, 0, 0, 0, 0, 0, 5, 1));
        CHECK(testInspection.get_TCP_packet_rate() == 10);
        CHECK(testInspection.get_attack_level() == 5);
        CHECK(testInspection.get_TCP_threshold() == 0);
    }
    SECTION("send to global Statisic", "[]") {
        // sending to global statistic not implemented yet in main
    }
}