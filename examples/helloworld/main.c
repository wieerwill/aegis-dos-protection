
/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */
#include <errno.h>
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
#include <rte_cycles.h>

static int lcore_hello(__rte_unused void *arg) {
    unsigned lcore_id;
    lcore_id = rte_lcore_id();
    printf("hello from core %u\n", lcore_id);
    return 0;
}

int main(int argc, char **argv) {
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
    uint32_t seconds = 0;
    uint64_t cycles_old = rte_get_tsc_cycles();
    for (;;) {
        uint64_t hz = rte_get_tsc_hz();
        uint64_t cycles = rte_get_tsc_cycles();
        uint64_t delta_cycles = cycles - cycles_old;

        if (delta_cycles >= hz) {
            ++seconds;
            cycles_old = cycles - (delta_cycles % hz); 
        }

        printf("seconds : %u\n", seconds);
    }

    rte_eal_mp_wait_lcore();

    /* clean up the EAL */
    rte_eal_cleanup();
    return 0;
}

uint64_t int_divide(uint64_t a, uint64_t b) {
    uint64_t c = 0;
    while (a >= b) {
        a -= b;
        ++c;
    }

    return c;
}