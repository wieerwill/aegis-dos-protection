#pragma once

#include <rte_ring_core.h>
#include <rte_ring_elem.h>

#include <string>

#include "Threads/Thread.hpp"

struct Stats {
    uint64_t attacks;
    uint64_t bytes;
    uint64_t dropped;
    uint64_t packets;
    uint64_t work_time;
    uint64_t syn_level;

    Stats* operator+=(const Stats* new_stats);
};

struct StatsMonitor {
    double attacks_per_second;
    double attacks_percent;
    double bytes_per_second;
    double dropped_per_second;
    double dropped_percent;
    double packets_per_second;
    double proc_speed; // process speed
    double total_time;
};

class StatisticsThread : public Thread {
  public:
    static int s_run(void* thread_vptr);

    void enqueue_statistics(int& id, Stats* new_stats);
    void update_statistics(Stats* new_stats);

  private:
    void run();
    static rte_ring* _s_queue[16];
    static Stats* _s_stats;
};
