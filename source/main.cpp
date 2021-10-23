#include <signal.h>

#include "ConfigurationManagement/Configurator.hpp"
#include "Initializer.hpp"
#include "PacketDissection/PacketContainer.hpp"
#include "Threads/DefenseThread.hpp"

void handle_quit(int signum);
void terminate(DefenseThread** thread_arr, uint16_t nb_worker_threads);

bool quit = false;

int main(int argc, char** argv) {

    // ===== INITIALIZE ===== //
    // Register signal and signal handler
    signal(SIGINT, handle_quit);

    Configurator::instance()->read_config("../config.json");

    Initializer* init = new Initializer();
    unsigned int lcore_id;
    uint16_t inside_port = 1;
    uint16_t outside_port = 0;
    uint16_t nb_worker_threads = 0;

    struct rte_mempool* mbuf_pool =
        init->init_dpdk(argc, argv, nb_worker_threads);

    // create thread objects
    DefenseThread* thread_arr[nb_worker_threads];
    MbufContainerReceiving* pkt_containers_from_outside[nb_worker_threads];
    MbufContainerReceiving* pkt_containers_from_inside[nb_worker_threads];
    MbufContainerTransmitting* pkt_containers_to_inside[nb_worker_threads];
    MbufContainerTransmitting* pkt_containers_to_outside[nb_worker_threads];

    for (int i = 0; i < nb_worker_threads; i++) {

        pkt_containers_from_outside[i] =
            new MbufContainerReceiving(mbuf_pool, outside_port, i);
        pkt_containers_from_inside[i] =
            new MbufContainerReceiving(mbuf_pool, inside_port, i);
        pkt_containers_to_inside[i] =
            new MbufContainerTransmitting(mbuf_pool, inside_port, i);
        pkt_containers_to_outside[i] =
            new MbufContainerTransmitting(mbuf_pool, outside_port, i);

        thread_arr[i] = new DefenseThread(
            pkt_containers_from_inside[i], pkt_containers_from_outside[i],
            pkt_containers_to_inside[i], pkt_containers_to_outside[i]);
    }

    // start each thread on an own lcore
    lcore_id = rte_lcore_id();

    for (int i = 0; i < nb_worker_threads; ++i) {
        lcore_id = rte_get_next_lcore(lcore_id, true, true);

        rte_eal_remote_launch(
            static_cast<lcore_function_t*>(DefenseThread::s_run), thread_arr[i],
            lcore_id);
    }

    /*
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
           */

    // =================== START CLI ==================== //

    std::string input = "";
    const std::string NAME = "aegis";

    enum State { RUNNING, IDLE };
    State state = IDLE;

    while (input != "exit") {

        std::cout << std::endl;
        std::cout << NAME << "> " << std::flush;
        std::cin >> input;
        std::cout << std::endl;

        if (input == "start") {
            if (state == IDLE) {

                for (int i = 0; i < nb_worker_threads; ++i) {
                    thread_arr[i]->start_treat();
                }

                state = RUNNING;

            } else { // state == RUNNING
                std::cout << "Cannot start if program is already running."
                          << std::endl;
            }

        } else if (input == "stop") {
            if (state == IDLE) {
                std::cout << "Cannot stop if program is not running."
                          << std::endl;

            } else { // state == RUNNING

                for (int i = 0; i < nb_worker_threads; ++i) {
                    thread_arr[i]->stop_treat();
                }

                state = IDLE;
            }

        } else if (input == "exit") {

            // do nothing; while loop stops

        } else if (input == "help" || input == "h") {
            std::cout << "start\tstart " << NAME << std::endl
                      << "stop\tstop " << NAME << std::endl
                      << "help, h\tprint commands " << std::endl
                      << "exit\texit " << NAME << std::endl
                      << std::endl;
        } else {
            std::cout << "Command unknown. Try 'h' or 'help'." << std::endl;
        }
    }

    // ==================== END CLI ==================== //

    // ===== TERMINATE ===== //
    terminate(thread_arr, nb_worker_threads);

    // destruct objects on heap
    for (int i = 0; i < nb_worker_threads; ++i) {
        delete thread_arr[i];
        thread_arr[i] = nullptr;

        delete pkt_containers_from_inside[i];
        pkt_containers_from_inside[i] = nullptr;

        delete pkt_containers_from_outside[i];
        pkt_containers_from_outside[i] = nullptr;

        delete pkt_containers_to_inside[i];
        pkt_containers_to_inside[i] = nullptr;

        delete pkt_containers_to_inside[i];
        pkt_containers_to_inside[i] = nullptr;
    }

    delete init;
    init = nullptr;

    // cleanup eal
    rte_eal_mp_wait_lcore();
    rte_eal_cleanup();

    // YOU ARE TERMINATED
}

void handle_quit(int signum) {
    // do nothing
    // quit = true;
}
//

void terminate(DefenseThread** thread_arr, uint16_t nb_worker_threads) {
    std::cout << "\nterminating..." << std::endl;

    for (int i = 0; i < nb_worker_threads; ++i) {
        thread_arr[i]->quit();
    }

    // wait for threads to end
    for (int i = 0; i < nb_worker_threads - 1; ++i) {
        while (thread_arr[i]->is_running()) {
            // wait
        }
    }
}