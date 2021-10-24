#include "Threads/StatisticsThread.hpp"
#include <iostream>

//-------------------------------------------------------------------------
//-------------------------- StatisticsThread -----------------------------
//-------------------------------------------------------------------------

StatisticsThread::StatisticsThread() {
    std::string name;
    unsigned int lcore_id = rte_lcore_id();
    for (int i = 0; i < 16; ++i) {
        lcore_id = rte_get_next_lcore(lcore_id, true, true);

        name = "RTE_RING_THREAD_";
        name += i + 1;
        const char* c =
            name.c_str(); // wrap string to char pointer //TODO: good name

        lcore_id = rte_get_next_lcore(lcore_id, true, true);
        _s_queue[i] = rte_ring_create(c, 4, rte_lcore_to_socket_id(lcore_id),
                                      RING_F_SP_ENQ | RING_F_SC_DEQ);

        // checking for errors in creating rte rings
        if (_s_queue[i] == NULL) {
            if (rte_errno == E_RTE_NO_CONFIG) {
                std::cout << "Error during RTE_RING creation at Ring " << i + 1
                          << ", return Value = E_RTE_NO_CONFIG" << std::endl;
            }
            if (rte_errno == E_RTE_SECONDARY) {
                std::cout << "Error during RTE_RING creation at Ring " << i + 1
                          << ", return Value = E_RTE_SECONDARY" << std::endl;
            }
            if (rte_errno == EINVAL) {
                std::cout << "Error during RTE_RING creation at Ring " << i + 1
                          << ", return Value = EINVAL" << std::endl;
            }
            if (rte_errno == ENOSPC) {
                std::cout << "Error during RTE_RING creation at Ring " << i + 1
                          << ", return Value = ENOSPC" << std::endl;
            }
            if (rte_errno == EEXIST) {
                std::cout << "Error during RTE_RING creation at Ring " << i + 1
                          << ", return Value = EEXIST" << std::endl;
            }
            if (rte_errno == ENOMEM) {
                std::cout << "Error during RTE_RING creation at Ring " << i + 1
                          << ", return Value = ENOMEM" << std::endl;
            }
        }
    } // rte_lcore_to_socket_id
    _s_stats_monitor = new StatsMonitor;
    _s_stats = new Stats;
    _s_stats_monitor->total_time = 0;
}

rte_ring* StatisticsThread::_s_queue[16];

int StatisticsThread::s_run(void* thread_vptr) {
    StatisticsThread* thread = static_cast<StatisticsThread*>(thread_vptr);
    thread->run();
    return 0;
}

void StatisticsThread::run() {
    // Check for new statistics in each rte_ring
    std::cout << "\n StatisticThread running on lcore " << rte_lcore_id()
              << ". [Ctrl+C to quit]\n"
              << std::endl;
    cycles_old = rte_get_tsc_cycles();
    while (likely(_quit == false)) {
        // Timer -> update_statistics_monitor
        if (timer_get_seconds() >=
            1) { // every second updating the stats monitor
            timer_reset();
            update_statistics_monitor();
        }
        for (struct rte_ring* ring : _s_queue) {
            // get stats from each Thread and put them into _s_stats
            Stats* out;
            if (rte_ring_dequeue(ring, (void**)&out) ==
                0) {               // taking existing stats from ring
                *_s_stats += *out; // updating _s_stats

                /*delete out;
                out = nullptr;*/
            }
        }
    }
}

void StatisticsThread::enqueue_statistics(const unsigned int& id,
                                          Stats* new_stats) {
    // enqueue statistics (checked and working)
    rte_ring_enqueue(_s_queue[id], (void*)new_stats);
}

void StatisticsThread::update_statistics_monitor() {
    // called once per second
    _s_stats_monitor->attacks_per_second = _s_stats->attacks;
    _s_stats_monitor->attacks_percent =
        ((double)_s_stats->attacks / (double)_s_stats->packets) * 100;
    _s_stats_monitor->bytes_per_second = _s_stats->bytes;
    _s_stats_monitor->dropped_per_second = _s_stats->dropped;
    _s_stats_monitor->attacks_percent =
        ((double)_s_stats->dropped / (double)_s_stats->packets) * 100;
    _s_stats_monitor->packets_per_second = _s_stats->packets;
    //_s_stats_monitor->process_speed = ;
    ++_s_stats_monitor->total_time;

    // delete old _s_stats
    _s_stats->reset_stats();
}

void StatisticsThread::print_stats_monitor(){

    std::cout << "Total runtime = " << _s_stats_monitor->total_time << "s" << std::endl;
    std::cout << "Bytes per second  = " << _s_stats_monitor->bytes_per_second << std::endl;
    std::cout << "Packets per second = " << _s_stats_monitor->packets_per_second << std::endl;
    std::cout << "Attacks per Second = " << _s_stats_monitor->attacks_per_second << std::endl;
    std::cout << "Attacks in percent = " << _s_stats_monitor->attacks_percent << "%" << std::endl;
    std::cout << "Dropped packets per second = " << _s_stats_monitor->dropped_per_second << std::endl;
    std::cout << "Dropped packets in percent = " << _s_stats_monitor->dropped_percent << "%" << std::endl;
    std::cout << "Prozess speed = " << _s_stats_monitor->process_speed << std::endl;
}

uint64_t StatisticsThread::timer_get_seconds() {

    cycles = rte_get_tsc_cycles();
    hz = rte_get_tsc_hz();

    // calculate
    seconds = (cycles - cycles_old) / hz;

    return seconds;
}

void StatisticsThread::timer_reset() {
    seconds = 0;
    cycles_old = cycles;
}

//-------------------------------------------------------------------------
//------------------------------- Stats -----------------------------------
//-------------------------------------------------------------------------

Stats& Stats::operator+=(const Stats& new_stats) {
    attacks += new_stats.attacks;
    bytes += new_stats.bytes;
    dropped += new_stats.dropped;
    packets += new_stats.packets;
    work_time += new_stats.work_time;
}
void Stats::reset_stats() {
    attacks = 0;
    bytes = 0;
    dropped = 0;
    packets = 0;
    work_time = 0;
}