#include <catch2/catch.hpp>

#include <boost/log/trivial.hpp>
#include <rte_mbuf.h>

#include "Definitions.hpp"
#include "PacketDissection/PacketContainer.hpp"
#include "PacketDissection/PacketInfo.hpp"
#include "PacketDissection/PacketInfoCreator.hpp"
#include "PacketDissection/PacketInfoIpv4Icmp.hpp"
#include "PacketDissection/PacketInfoIpv4Tcp.hpp"
#include "PacketDissection/PacketInfoIpv4Udp.hpp"
#include "PacketDissection/PacketInfoIpv6Icmp.hpp"
#include "PacketDissection/PacketInfoIpv6Tcp.hpp"
#include "PacketDissection/PacketInfoIpv6Udp.hpp"

TEST_CASE("PacketContainer", "[]") {
    // ===  I N I T  === //
    uint16_t inside_port = 0;
    uint16_t outside_port = 1;
    struct rte_mempool mbuf_pool_struct;
    struct rte_mempool* mbuf_pool = &mbuf_pool_struct;
    CHECK(mbuf_pool != nullptr);

    PacketContainer* pkt_container =
        new PacketContainer(mbuf_pool, inside_port, outside_port, 0, 0);
    CHECK(pkt_container != nullptr);

    // ===  S E C T I O N S  == //
    SECTION("get_empty_packet", "[]") {

        SECTION("default", "[]") {
            CHECK(pkt_container->get_number_of_polled_packets() == 0);
            CHECK(pkt_container->get_total_number_of_packets() == 0);

            PacketInfo* pkt_info = pkt_container->get_empty_packet();
            CHECK(pkt_info != nullptr);
            CHECK(pkt_info->get_mbuf() != nullptr);
            CHECK(pkt_info->get_type() == IPv4TCP);

            CHECK(pkt_container->get_total_number_of_packets() >=
                  pkt_container->get_number_of_polled_packets());
            CHECK(pkt_container->get_total_number_of_packets() == 1);
            CHECK(pkt_container->get_number_of_polled_packets() == 0);
        }

        SECTION("IPv4TCP", "[]") {
            CHECK(pkt_container->get_number_of_polled_packets() == 0);
            CHECK(pkt_container->get_total_number_of_packets() == 0);

            PacketInfo* pkt_info = pkt_container->get_empty_packet(IPv4TCP);
            CHECK(pkt_info != nullptr);
            CHECK(pkt_info->get_mbuf() != nullptr);
            CHECK(pkt_info->get_type() == IPv4TCP);

            CHECK(pkt_container->get_total_number_of_packets() >=
                  pkt_container->get_number_of_polled_packets());
            CHECK(pkt_container->get_total_number_of_packets() == 1);
            CHECK(pkt_container->get_number_of_polled_packets() == 0);
        }
    }

    SECTION("create more packets than burst size", "[]") {

        SECTION("fill till BURST_SIZE", "[]") {
            for (int i = 0; i < BURST_SIZE; ++i) {
                PacketInfo* pkt_info = pkt_container->get_empty_packet();
                CHECK(pkt_info != nullptr);
            }

            CHECK(pkt_container->get_total_number_of_packets() == BURST_SIZE);
        }

        SECTION("fill till BURST_SIZE + 1", "[]") {
            for (int i = 0; i < BURST_SIZE + 1; ++i) {
                PacketInfo* pkt_info = pkt_container->get_empty_packet();
                CHECK(pkt_info != nullptr);
            }

            CHECK(pkt_container->get_total_number_of_packets() ==
                  BURST_SIZE + 1);
        }

        CHECK(pkt_container->get_total_number_of_packets() >=
              pkt_container->get_number_of_polled_packets());
    }

    SECTION("get_packet_at_index", "[]") {

        SECTION("general", "[]") {
            // add empty packet
            PacketInfo* pkt_info_0 = pkt_container->get_empty_packet();
            CHECK(pkt_container->get_number_of_polled_packets() == 0);
            CHECK(pkt_container->get_total_number_of_packets() == 1);

            PacketInfo* pkt_info_1 = pkt_container->get_packet_at_index(
                pkt_container->get_total_number_of_packets() - 1);
            CHECK(pkt_info_0 == pkt_info_1);
            CHECK(pkt_info_1 != nullptr);
            CHECK(pkt_info_1->get_mbuf() != nullptr);
            CHECK(pkt_info_1->get_type() == IPv4TCP);

            CHECK(pkt_container->get_total_number_of_packets() >=
                  pkt_container->get_number_of_polled_packets());
            CHECK_NOTHROW(pkt_container->get_packet_at_index(
                pkt_container->get_total_number_of_packets() - 1));
            CHECK_THROWS(pkt_container->get_packet_at_index(
                pkt_container->get_total_number_of_packets()));
        }

        SECTION("test out of bounds error", "[]") {
            for (int i = 0; i < int(BURST_SIZE / 2); ++i) {
                pkt_container->get_empty_packet();
            }

            CHECK(pkt_container->get_total_number_of_packets() ==
                  int(BURST_SIZE / 2));

            for (int i = 0; i < int(BURST_SIZE / 2); ++i) {
                CHECK_NOTHROW(pkt_container->get_packet_at_index(i));
            }

            for (int i = int(BURST_SIZE / 2); i < BURST_SIZE; ++i) {
                CHECK_THROWS(pkt_container->get_packet_at_index(i));
            }

            CHECK_THROWS(pkt_container->get_packet_at_index(
                pkt_container->get_total_number_of_packets()));
            CHECK_NOTHROW(pkt_container->get_packet_at_index(
                pkt_container->get_total_number_of_packets() - 1));
        }
    }

    SECTION("take_packet and add_packet", "[]") {

        PacketInfo* pkt_info_0 = pkt_container->get_empty_packet();
        CHECK(pkt_container->get_number_of_polled_packets() == 0);
        CHECK(pkt_container->get_total_number_of_packets() == 1);

        PacketInfo* pkt_info_1 = pkt_container->take_packet(0);
        CHECK(pkt_info_0 == pkt_info_1);
        CHECK(pkt_info_1 != nullptr);
        CHECK(pkt_info_1->get_mbuf() != nullptr);
        CHECK(pkt_container->get_packet_at_index(0) == nullptr);
        CHECK(pkt_container->get_total_number_of_packets() == 1);
        CHECK(pkt_container->get_total_number_of_packets() >=
              pkt_container->get_number_of_polled_packets());

        pkt_container->add_packet(pkt_info_1);
        CHECK(pkt_container->get_total_number_of_packets() >=
              pkt_container->get_number_of_polled_packets());
        CHECK(pkt_container->get_number_of_polled_packets() == 0);
        CHECK(pkt_container->get_total_number_of_packets() == 2);
        CHECK(pkt_container->get_packet_at_index(1) == pkt_info_1);
        CHECK(pkt_container->get_packet_at_index(0) == nullptr);
    }

    SECTION("drop_packet", "[]") {

        SECTION("default") {
            PacketInfo* pkt_info_0 = pkt_container->get_empty_packet();
            CHECK(pkt_container->get_number_of_polled_packets() == 0);
            CHECK(pkt_container->get_total_number_of_packets() == 1);

            pkt_container->drop_packet(0);
            CHECK(pkt_container->get_total_number_of_packets() >=
                  pkt_container->get_number_of_polled_packets());
            CHECK(pkt_container->get_number_of_polled_packets() == 0);
            CHECK(pkt_container->get_total_number_of_packets() == 1);
            CHECK(pkt_container->get_packet_at_index(0) == nullptr);

            CHECK_NOTHROW(pkt_container->drop_packet(0));
            CHECK(pkt_container->get_number_of_polled_packets() == 0);
            CHECK(pkt_container->get_total_number_of_packets() == 1);
            CHECK(pkt_container->get_packet_at_index(0) == nullptr);
        }
    }

    SECTION("poll_packets", "[]") {

        CHECK(pkt_container->get_number_of_polled_packets() == 0);
        CHECK(pkt_container->get_total_number_of_packets() == 0);

        uint16_t nb_pkts_polled;
        pkt_container->poll_packets(nb_pkts_polled);
        CHECK(pkt_container->get_number_of_polled_packets() > 0);
        CHECK(pkt_container->get_total_number_of_packets() ==
              pkt_container->get_number_of_polled_packets());
        CHECK(nb_pkts_polled == pkt_container->get_number_of_polled_packets());

        CHECK_NOTHROW(pkt_container->get_packet_at_index(nb_pkts_polled - 1));
        PacketInfo* pkt_info =
            pkt_container->get_packet_at_index(nb_pkts_polled - 1);
        CHECK(pkt_info != nullptr);
        CHECK(pkt_info->get_mbuf() != nullptr);
    }

    SECTION("send_packets", "[]") {

        SECTION("do not drop") {
            SECTION("send created packets") {
                PacketInfo* pkt_info = pkt_container->get_empty_packet();
                CHECK(pkt_container->get_number_of_polled_packets() == 0);
                CHECK(pkt_container->get_total_number_of_packets() == 1);
            }

            SECTION("send polled packets") {
                uint16_t nb_polled_pkts;
                pkt_container->poll_packets(nb_polled_pkts);

                CHECK(nb_polled_pkts > 0);
            }

            SECTION("send polled and created packets") {
                uint16_t nb_polled_pkts;
                pkt_container->poll_packets(nb_polled_pkts);
                CHECK(nb_polled_pkts > 0);

                PacketInfo* pkt_info = pkt_container->get_empty_packet();
                CHECK(pkt_container->get_total_number_of_packets() >
                      pkt_container->get_number_of_polled_packets());
            }

            pkt_container->send_packets();
            CHECK(pkt_container->get_number_of_polled_packets() == 0);
            CHECK(pkt_container->get_total_number_of_packets() == 0);
        }

        SECTION("drop and send created packets") {

            const int PKTS_TO_POLL = 4;
            int nb_pkts_to_drop = -1;

            for (int i = 0; i < PKTS_TO_POLL; ++i) {
                pkt_container->get_empty_packet();
            }

            CHECK(pkt_container->get_total_number_of_packets() >
                  pkt_container->get_number_of_polled_packets());
            CHECK(pkt_container->get_total_number_of_packets() == PKTS_TO_POLL);

            SECTION("drop first packet") {
                nb_pkts_to_drop = 1;
                CHECK(pkt_container->get_nb_mbufs_in_mbuf_arr()[0] ==
                      PKTS_TO_POLL);
                CHECK(pkt_container->get_nb_pkts_dropped() == 0);
                pkt_container->drop_packet(0);
                CHECK(pkt_container->get_nb_pkts_dropped() == 1);
            }

            SECTION("drop second packet") {
                nb_pkts_to_drop = 1;
                CHECK(pkt_container->get_nb_mbufs_in_mbuf_arr()[0] ==
                      PKTS_TO_POLL);
                CHECK(pkt_container->get_nb_pkts_dropped() == 0);
                pkt_container->drop_packet(1);
                CHECK(pkt_container->get_nb_pkts_dropped() == 1);
            }

            SECTION("drop second and third packet") {
                nb_pkts_to_drop = 2;
                CHECK(pkt_container->get_nb_mbufs_in_mbuf_arr()[0] ==
                      PKTS_TO_POLL);
                CHECK(pkt_container->get_nb_pkts_dropped() == 0);
                pkt_container->drop_packet(1);
                pkt_container->drop_packet(2);
                CHECK(pkt_container->get_nb_pkts_dropped() == 2);
            }

            SECTION("drop last packet") {
                nb_pkts_to_drop = 1;
                CHECK(pkt_container->get_nb_mbufs_in_mbuf_arr()[0] ==
                      PKTS_TO_POLL);
                CHECK(pkt_container->get_nb_pkts_dropped() == 0);
                pkt_container->drop_packet(PKTS_TO_POLL - 1);
                CHECK(pkt_container->get_nb_pkts_dropped() == 1);
            }

            pkt_container->reorder_mbuf_arrays();
            CHECK(pkt_container->get_nb_mbufs_in_mbuf_arr()[0] ==
                  PKTS_TO_POLL - nb_pkts_to_drop);

            pkt_container->send_packets();
            CHECK(pkt_container->get_number_of_polled_packets() == 0);
            CHECK(pkt_container->get_total_number_of_packets() == 0);
        }
    }

    // ===  D E L E T E   O B J E C T S  === //
    delete pkt_container;
    pkt_container = nullptr;
}
