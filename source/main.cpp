#include <signal.h>

#include "Configurator.hpp"
#include "Initializer.hpp"
#include "PacketDissection/PacketContainer.hpp"
#include "Threads/DefenseThread.hpp"
#include "Threads/StatisticsThread.hpp"

int main(int argc, char** argv);
void handle_quit(int signum);

bool quit = false;

int main(int argc, char** argv) {

    // ===== INITIALIZE ===== //
    // Register signal and signal handler
    signal(SIGINT, handle_quit);

    Configurator::instance()->read_config("../config.json");

    Initializer* init = new Initializer();
    unsigned int lcore_id;
    uint16_t inside_port = 0;
    uint16_t outside_port = 1;
    uint16_t nb_worker_threads = 0;

    struct rte_mempool* mbuf_pool =
        init->init_dpdk(argc, argv, nb_worker_threads);

    // create thread objects

    StatisticsThread* stat_thread = new StatisticsThread();
    DefenseThread* thread_arr[nb_worker_threads];
    PacketContainer* pkt_containers_to_outside[nb_worker_threads];
    PacketContainer* pkt_containers_to_inside[nb_worker_threads];
    for (int i = 0; i < nb_worker_threads; i++) {

        pkt_containers_to_outside[i] =
            new PacketContainer(mbuf_pool, inside_port, outside_port, i, i);
        pkt_containers_to_inside[i] =
            new PacketContainer(mbuf_pool, outside_port, inside_port, i, i);

        thread_arr[i] = new DefenseThread(pkt_containers_to_outside[i],
                            pkt_containers_to_inside[i], stat_thread);
    }


    // start DefenseThreads
    // ..start each thread on an own lcore
    lcore_id = rte_lcore_id();
    for (int i = 0; i < nb_worker_threads; ++i) {
        lcore_id = rte_get_next_lcore(lcore_id, true, true);

        rte_eal_remote_launch(
            static_cast<lcore_function_t*>(DefenseThread::s_run), thread_arr[i],
            lcore_id);
    }

    uint64_t hz = rte_get_tsc_hz();
    u_int64_t cycles = rte_get_tsc_cycles();
    u_int64_t old_cycles = cycles;
    while (likely(quit == false)) {
        cycles = rte_get_tsc_cycles();
        if (cycles - old_cycles > 64 * hz) {
            Treatment::s_increment_timestamp();
            old_cycles = cycles;
        }
    }

    // ===== TERMINATE ===== //
    std::cout << "\nterminating..." << std::endl;

    for (int i = 0; i < nb_worker_threads; ++i) {
        thread_arr[i]->quit();
    }

    stat_thread->quit();

    // destruct objects on heap
    for (int i = 0; i < nb_worker_threads; ++i) {
        delete thread_arr[i];
        thread_arr[i] = nullptr;

        delete pkt_containers_to_inside[i];
        pkt_containers_to_inside[i] = nullptr;

        delete pkt_containers_to_outside[i];
        pkt_containers_to_outside[i] = nullptr;
    }

    delete stat_thread;
    stat_thread = nullptr;

    delete init;
    init = nullptr;

    // cleanup eal
    rte_eal_mp_wait_lcore();
    rte_eal_cleanup();

    // YOU ARE TERMINATED
}

void handle_quit(int signum) {
    //
    quit = true;
}
//