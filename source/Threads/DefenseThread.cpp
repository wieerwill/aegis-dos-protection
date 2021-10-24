#include <iostream>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#include "Definitions.hpp"
#include "Threads/DefenseThread.hpp"
#include "Threads/StatisticsThread.hpp"

// ===== PUBLIC ===== //

DefenseThread::DefenseThread(PacketContainer* pkt_container_to_inside,
                             PacketContainer* pkt_container_to_outside,
                             StatisticsThread* stat_thread)
    : ForwardingThread(pkt_container_to_inside, pkt_container_to_outside),
      _treatment(pkt_container_to_inside, pkt_container_to_outside),
      _statistics_thread(stat_thread) {}

int DefenseThread::s_run(void* thread_vptr) {
    DefenseThread* thread = static_cast<DefenseThread*>(thread_vptr);
    thread->run();
    return 0;
}

// ===== PRIVATE ===== //

void DefenseThread::run() {
    uint16_t nb_pkts_to_inside;
    uint16_t nb_pkts_to_outside;

    BOOST_LOG_TRIVIAL(info)
        << "\nRunning on lcore " << rte_lcore_id() << ". [Ctrl+C to quit]\n";

    // stats for StatisticThread_testing
    Stats* thread_statistics = new Stats();
    thread_statistics->attacks = rte_lcore_id();
    thread_statistics->bytes = rte_lcore_id();
    thread_statistics->dropped = rte_lcore_id();
    thread_statistics->packets = rte_lcore_id();
    thread_statistics->work_time = rte_lcore_id();

    // Run until the application is quit or killed.
    while (likely(_quit == false)) {
        _statistics_thread->enqueue_statistics(rte_lcore_id(),
                                               thread_statistics);

        // ===== ALICE --[DAVE]--> BOB ===== //

        // continue if no packets are received
        _pkt_container_to_inside->poll_packets(nb_pkts_to_inside);
        if (likely(nb_pkts_to_inside > 0)) {

            /// TODO: implement pipeline

            _treatment.treat_packets_to_inside();
            _pkt_container_to_inside->send_packets();
            _pkt_container_to_outside->send_packets();
        }
        // ===== ALICE <--[DAVE]-- BOB ===== //

        // continue if no packets are received
        _pkt_container_to_outside->poll_packets(nb_pkts_to_outside);
        if (likely(nb_pkts_to_outside > 0)) {

            /// TODO: implement pipeline
            _treatment.treat_packets_to_outside();
            _pkt_container_to_inside->send_packets();
            _pkt_container_to_outside->send_packets();
        }

        if (likely(nb_pkts_to_inside != 0 || nb_pkts_to_outside != 0)) {
            BOOST_LOG_TRIVIAL(info)
                << "pkts_to_inside = " << nb_pkts_to_inside
                << "\tpkts_to_outside = " << nb_pkts_to_outside << "\n";
        }
    }

    _running = false;
}
