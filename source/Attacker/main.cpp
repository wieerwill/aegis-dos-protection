#include <chrono>
#include <iostream>
#include <signal.h>

#include "Configurator.hpp"
#include "Initializer.hpp"
#include "PacketDissection/PacketContainer.hpp"
#include "Threads/AttackThread.hpp"

int main(int argc, char** argv);
void handle_quit(int signum);

bool quit = false;

int main(int argc, char** argv) {

    // ===== INITIALIZE ===== //
    // Register signal and signal handler
    signal(SIGINT, handle_quit);

    Configurator::instance()->read_config("../test/Attacker_config.json");

    Initializer* init = new Initializer();
    unsigned int lcore_id;
    uint16_t dave_port = 0;
    uint16_t alice_port = 1;
    uint16_t nb_worker_threads = 0;

    struct rte_mempool* mbuf_pool =
        init->init_dpdk_attacker(argc, argv, nb_worker_threads);

    // create thread objects
    AttackThread* thread_arr[nb_worker_threads];
    PacketContainerLean* pkt_containers_to_alice[nb_worker_threads];
    PacketContainerLean* pkt_containers_to_dave[nb_worker_threads];
    for (int i = 0; i < nb_worker_threads; i++) {

        pkt_containers_to_alice[i] =
            new PacketContainerLean(mbuf_pool, dave_port, alice_port, i, i);
        pkt_containers_to_dave[i] =
            new PacketContainerLean(mbuf_pool, alice_port, dave_port, i, i);

        thread_arr[i] =
            new AttackThread(pkt_containers_to_alice[i],
                             pkt_containers_to_dave[i], nb_worker_threads);
    }

    // start each thread on an own lcore
    lcore_id = rte_lcore_id();

    // run every thread except the last one
    for (int i = 0; i < nb_worker_threads - 1; ++i) {
        lcore_id = rte_get_next_lcore(lcore_id, true, true);

        rte_eal_remote_launch(
            static_cast<lcore_function_t*>(AttackThread::s_run), thread_arr[i],
            lcore_id);
    }
    // start measuring time running
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    // ===== RUN ===== //
    std::cout << "\nRunning on lcore " << rte_lcore_id()
              << ". [Ctrl+C to quit]\n"
              << std::endl;

    while (quit == false) {
        thread_arr[nb_worker_threads - 1]->iterate();
#ifdef SINGLE_ITERATION
        quit = true;
#endif
    }

    // ===== TERMINATE ===== //
    std::cout << "\nterminating..." << std::endl;

    for (int i = 0; i < nb_worker_threads - 1; ++i) {
#ifndef SINGLE_ITERATION
        thread_arr[i]->quit();
#endif
    }
    // stop measuring time running
    end = std::chrono::system_clock::now();
    // calculate total number of attack packets
    uint64_t nb_atk_pkts = 0;
    uint64_t nb_pkts_to_alice = 0;
    uint64_t nb_pkts_from_alice = 0;
    uint64_t data_volume_to_alice = 0;
    uint64_t data_volume_from_alice = 0;
    int pkt_type_used = 0;

    // wait for threads to end
    for (int i = 0; i < nb_worker_threads - 1; ++i) {
        while (thread_arr[i]->is_running()) {
            // wait
        }
    }

    // destruct objects on heap
    for (int i = 0; i < nb_worker_threads; ++i) {
        // rescue Informations
        nb_atk_pkts += thread_arr[i]->get_total_nb_atk_pkts();
        nb_pkts_to_alice += thread_arr[i]->get_total_nb_pkts_to_alice();
        nb_pkts_from_alice += thread_arr[i]->get_total_nb_pkts_from_alice();
        data_volume_to_alice += thread_arr[i]->get_total_data_volume_to_alice();
        data_volume_from_alice +=
            thread_arr[i]->get_total_data_volume_from_alice();
        pkt_type_used += thread_arr[i]->get_atk_pkt_type();

        delete thread_arr[i];
        thread_arr[i] = nullptr;

        delete pkt_containers_to_dave[i];
        pkt_containers_to_dave[i] = nullptr;

        delete pkt_containers_to_alice[i];
        pkt_containers_to_alice[i] = nullptr;
    }

    // print attack statistics
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "\nduration of attack:\t\t\t\t" << elapsed_seconds.count()
              << " seconds" << std::endl;
    int pkt_size = -1;
    if (pkt_type_used > 1) { // TCP: tcp + ip + ether
        pkt_size = 20 + 20 + 14;
    } else { // UDP: udp + ip + ether
        pkt_size = 8 + 20 + 14;
    }
    std::cout << "attack packets sent:\t\t\t\t" << nb_atk_pkts << std::endl;

    std::cout << "data volume sent (without L1 header):\t\t"
              << nb_atk_pkts * pkt_size / 1073741824 << " GByte" << std::endl;

    std::cout << "data volume sent (with L1 header):\t\t"
              << nb_atk_pkts * 84 / 1073741824 << " GByte" << std::endl;

    std::cout << "average attack rate (without L1 header):\t"
              << nb_atk_pkts * pkt_size / elapsed_seconds.count() / 1073741824 *
                     8
              << " Gbps" << std::endl;

    std::cout << "average attack rate (with L1 header):\t\t"
              << nb_atk_pkts * 84 / elapsed_seconds.count() / 1073741824 * 8
              << " Gbps" << std::endl;

    std::cout << "average attack packet rate:\t\t\t"
              << nb_atk_pkts / elapsed_seconds.count() / 1000000 << " Mpps"
              << std::endl;

    std::cout << "\nnumber packets sent from alice:\t\t\t" << nb_pkts_from_alice
              << std::endl;

    std::cout << "number packets recieved by alice:\t\t" << nb_pkts_to_alice
              << std::endl;

    std::cout << "average data rate sent from alice:\t\t"
              << data_volume_from_alice / elapsed_seconds.count() / 1024 * 8
              << " Kbps" << std::endl;

    std::cout << "average data rate recieved by alice:\t\t"
              << data_volume_to_alice / elapsed_seconds.count() / 1024 * 8
              << " Kbps" << std::endl
              << std::endl;

    delete init;
    init = nullptr;

    // cleanup eal
    rte_eal_mp_wait_lcore();
    rte_eal_cleanup();

    // YOU ARE TERMINATED
}

void handle_quit(int signum) { quit = true; }