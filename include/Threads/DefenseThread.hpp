/**
 * @file DefenseThread.hpp
 * @author Jakob
 * @brief 
 * @version 0.1
 * @date 2021-07-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include "PacketDissection/PacketContainer.hpp"
#include "Inspection.hpp"
#include "Treatment/Treatment.hpp"
#include "Threads/StatisticsThread.hpp"
#include "Threads/ForwardingThread.hpp"

class DefenseThread : public ForwardingThread {
  public:
    DefenseThread(PacketContainer* pkt_container_to_inside,
                  PacketContainer* pkt_container_to_outside,
                  StatisticsThread* stat_thread);

    /**
     * @brief Wrapper for the run-method
     *
     * Static method that calls the run method on a specific thread
     * object. This is necessary since s_run is used to start a new thread with
     * the dpdk function "rte_eal_remote_launch" which only takes specific types
     * of funktions.
     *
     * @param thread_obj object of type thread the run method is to be called on
     * @return int
     */
    static int s_run(void* thread_obj);

  private:
    Inspection _inspection;
    Treatment _treatment;
    StatisticsThread* _statistics_thread;
    /**
     * @brief Run thread
     *
     * "Main"-routine of a thread. It is executed until the application is quit
     * or killed.
     */
    void run();
};