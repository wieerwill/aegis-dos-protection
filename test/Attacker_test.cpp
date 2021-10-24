#include <catch2/catch.hpp>
#include <iostream>

#include <rte_cycles.h>

TEST_CASE("tsc timer", "[]") {
    // count and print seconds since test started
    // stop at 30 seconds
    // const std::string clear(100, '\n');
    const uint64_t MAX_SECONDS = 30;
    uint64_t cycles_old = 0;
    uint64_t cycles = 0;
    uint64_t hz = rte_get_tsc_hz();
    uint64_t seconds = 0;
    uint64_t delta_t = 0;

    // print initial message
    std::cout << "cycles : " << cycles << "\t"
              << "hz : " << hz << "\t"
              << "seconds : " << seconds << "\t" << std::endl;

    while (seconds < MAX_SECONDS) {
        cycles_old = cycles;
        cycles = rte_get_tsc_cycles();
        hz = rte_get_tsc_hz();

        // calculate
        delta_t = uint64_t(1 / hz * (cycles - cycles_old));
        seconds += delta_t;

        // print
        // std::cout << clear;
        std::cout << "cycles : " << cycles << "\t"
                  << "hz : " << hz << "\t"
                  << "seconds : " << seconds << "\t" << std::endl;
    }
}