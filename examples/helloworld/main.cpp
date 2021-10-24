
/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */
#include <chrono>
#include <cmath>
#include <errno.h>
#include <iostream>
#include <rte_cycles.h>
#include <rte_debug.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_lcore.h>
#include <rte_memory.h>
#include <rte_per_lcore.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/queue.h>

static int lcore_hello(__rte_unused void* arg) {
    unsigned lcore_id;
    lcore_id = rte_lcore_id();
    printf("hello from core %u\n", lcore_id);
    return 0;
}

int main(int argc, char** argv) {
    int socket_count;
    int lcore_count;
    int ret;
    unsigned lcore_id;
    ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_panic("Cannot init EAL\n");

    /* call lcore_hello() on every worker lcore */
    RTE_LCORE_FOREACH_WORKER(lcore_id) {
        rte_eal_remote_launch(lcore_hello, NULL, lcore_id);
    }

    /* call it on main lcore too */
    lcore_hello(NULL);

    // print socket count
    socket_count = rte_socket_count();
    printf("number of sockets:%i", socket_count);

    // print lcore count
    lcore_count = rte_lcore_count();
    printf("number of lcores:%i", lcore_count);

    // test timer
    int hz_mean = 0;
    int n = 1;
    int hz_cycles = -1;

    std::chrono::time_point<std::chrono::system_clock> start, end;

    start = std::chrono::system_clock::now();
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::chrono::seconds sec(30);

    do {
        hz_mean = (hz_mean + rte_get_tsc_hz() / n) * n / (n + 1);
        end = std::chrono::system_clock::now();
        elapsed_seconds = end - start;
    } while (elapsed_seconds <= sec);

    int cycles_old = rte_get_tsc_cycles();
    do {
        end = std::chrono::system_clock::now();
        elapsed_seconds = end - start;
    } while (elapsed_seconds <= sec);

    hz_cycles = (rte_get_tsc_cycles() - cycles_old) / 30;

    std::cout << std::endl << "hz : " << rte_get_tsc_hz() << std::endl;
    std::cout << "hz_cycles : " << hz_cycles << std::endl;
    std::cout << "hz_mean : " << hz_mean << std::endl;

    int length = 1000;
    int hz_cycles_diff[length];
    int hz_mean_diff[length];
    int hz_cycles_diff_max = -1;
    int hz_mean_diff_max = -1;
    int hz = -1;
    for (int i = 0; i < length; i++) {
        hz = rte_get_tsc_hz();

        hz_cycles_diff[i] = abs(hz - hz_cycles);
        hz_mean_diff[i] = abs(hz - hz_mean);

        if (hz_cycles_diff[i] > hz_cycles_diff_max) {
            hz_cycles_diff_max = hz_cycles_diff[i];
        }

        if (hz_mean_diff[i] > hz_mean_diff_max) {
            hz_mean_diff_max = hz_mean_diff[i];
        }
    }

    std::cout << "hz_cycles_diff_max : " << hz_cycles_diff_max << std::endl;
    std::cout << "hz_mean_diff_max : " << hz_mean_diff_max << std::endl;

    rte_eal_mp_wait_lcore();

    /* clean up the EAL */
    rte_eal_cleanup();
    return 0;
}