#include "Threads/StatisticsThread.hpp"

//-------------------------------------------------------------------------
//-------------------------- StatisticsThread -----------------------------
//-------------------------------------------------------------------------

rte_ring* StatisticsThread::_s_queue[16];

int StatisticsThread::s_run(void* thread_vptr) {
    StatisticsThread* thread = static_cast<StatisticsThread*>(thread_vptr);
    thread->run();
    return 0;
}

void StatisticsThread::run() {
    // Check for new statistics in each rte_ring
    while (_quit != false) {
        for (rte_ring* ring : _s_queue) {
            // Get stats from queue and update them
            Stats* out;
            rte_ring_dequeue(ring, (void**)&out);

            if (true) { // \TODO test if empty
                update_statistics(out);
            }
        }
    }
}

void StatisticsThread::enqueue_statistics(int& id, Stats* new_stats) {
    // enqueue statistics
    rte_ring* ring = _s_queue[id];
    rte_ring_enqueue(ring, new_stats);
}

Stats* StatisticsThread::_s_stats;

void StatisticsThread::update_statistics(Stats* new_stats) {
    //_s_stats += new_stats;
}

//-------------------------------------------------------------------------
//-------------------------- Stats ----------------------------------------
//-------------------------------------------------------------------------

Stats* Stats::operator+=(const Stats* new_stats) {
    this->attacks += new_stats->attacks;
    this->bytes += new_stats->bytes;
    this->dropped += new_stats->dropped;
    this->packets += new_stats->packets;
    this->work_time += new_stats->work_time;
    this->syn_level += new_stats->syn_level;

    return this;
}
