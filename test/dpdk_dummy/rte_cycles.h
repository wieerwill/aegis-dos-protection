#pragma once

#include <stdint.h>

uint64_t rte_get_tsc_cycles() {
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

uint64_t rte_get_tsc_hz() {
    // 1.8 GHz
    return 1800000000;
}
