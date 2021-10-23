#pragma once

#include "PacketDissection/PacketContainer.hpp"
#include "Treatment/Treatment.hpp"

#include "Threads/ForwardingThread.hpp"

class DefenseThread : public Thread {
  public:
    DefenseThread(MbufContainerReceiving* mbuf_container_from_inside,
                  MbufContainerReceiving* mbuf_container_from_outside,
                  MbufContainerTransmitting* mbuf_container_to_inside,
                  MbufContainerTransmitting* mbuf_container_to_outside);

    ~DefenseThread() { delete _treatment; }

    void start_treat() { _do_treat = true; }

    void stop_treat() { _do_treat = false; }

    bool _do_treat;

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
    Treatment* _treatment;

    MbufContainerReceiving* _mbuf_container_from_outside;
    MbufContainerReceiving* _mbuf_container_from_inside;
    MbufContainerTransmitting* _mbuf_container_to_outside;
    MbufContainerTransmitting* _mbuf_container_to_inside;

    /**
     * @brief Run thread
     *
     * "Main"-routine of a thread. It is executed until the application is quit
     * or killed.
     */
    void run();
};