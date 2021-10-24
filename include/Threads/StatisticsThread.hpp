/**
 * @file StatisticsThread.hpp
 * @author Jakob
 * @brief 
 * @version 0.1
 * @date 2021-07-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <rte_errno.h>
#include <rte_lcore.h>
#include <rte_ring.h>
//#include <rte_ring_core.h>
//#include <rte_ring_elem.h>
#include <string>

#include "Threads/Thread.hpp"
#include "Definitions.hpp"

struct Stats {
    uint64_t attacks;
    uint64_t bytes;
    uint64_t dropped;
    uint64_t packets;
    uint64_t work_time;

    Stats& operator+=(const Stats& new_stats);
    void reset_stats();
};

// all current stats which can be displayed
struct StatsMonitor {
    double attacks_per_second;
    double attacks_percent;
    double bytes_per_second;
    double dropped_per_second;
    double dropped_percent;
    double packets_per_second;
    double process_speed;
    double total_time;
};

class StatisticsThread : public Thread {
  public:
    StatisticsThread();
    static int s_run(void* thread_vptr);

    void enqueue_statistics(const unsigned int& id, Stats* new_stats);
    void update_statistics_monitor();
    void print_stats_monitor();

  private:
    void run();
    static rte_ring* _s_queue[16]; // todo : dynamic
    Stats* _s_stats;
    StatsMonitor* _s_stats_monitor;

    uint64_t timer_get_seconds();
    void timer_reset();

    // timer variables
    uint64_t cycles_old = 0;
    uint64_t cycles = 0;
    uint64_t seconds = 0;
    uint64_t delta_t = 0;
    uint64_t hz;
};
