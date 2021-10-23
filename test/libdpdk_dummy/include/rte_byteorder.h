#pragma once

#include <stdint.h>

#define rte_cpu_to_be_16(x) rte_bswap16(x)
#define rte_be_to_cpu_16(x) rte_bswap16(x)

#define rte_cpu_to_be_32(x) rte_bswap32(x)
#define rte_be_to_cpu_32(x) rte_bswap32(x)

#define rte_bswap16(x)                                                         \
    ((uint16_t)(__builtin_constant_p(x) ? rte_constant_bswap16(x)              \
                                        : rte_arch_bswap16(x)))

#define rte_bswap32(x)                                                         \
    ((uint32_t)(__builtin_constant_p(x) ? rte_constant_bswap32(x)              \
                                        : rte_arch_bswap32(x)))

typedef uint16_t rte_be16_t;

typedef uint32_t rte_be32_t;

/**
 * An internal function to swap bytes in a
 * 16-bit value.
 *
 * It is used by rte_bswap16() when the
 * value is constant. Do not use this
 * function directly; rte_bswap16() is
 * preferred.
 */
static inline uint16_t rte_constant_bswap16(uint16_t x) {
    return (uint16_t)(((x & 0x00ffU) << 8) | ((x & 0xff00U) >> 8));
}

/**
 * An internal function to swap bytes in a 32-bit value.
 *
 * It is used by rte_bswap32() when the value is constant. Do not use
 * this function directly; rte_bswap32() is preferred.
 */
static inline uint32_t rte_constant_bswap32(uint32_t x) {
    return ((x & 0x000000ffUL) << 24) | ((x & 0x0000ff00UL) << 8) |
           ((x & 0x00ff0000UL) >> 8) | ((x & 0xff000000UL) >> 24);
}

static inline uint16_t rte_arch_bswap16(uint16_t _x) {
    uint16_t x = _x;
    asm volatile("xchgb %b[x1],%h[x2]" : [ x1 ] "=Q"(x) : [ x2 ] "0"(x));
    return x;
}

/*
 * An architecture-optimized byte swap for a 32-bit value.
 *
 * Do not use this function directly. The preferred function is rte_bswap32().
 */
static inline uint32_t rte_arch_bswap32(uint32_t _x) {
    uint32_t x = _x;
    asm volatile("bswap %[x]" : [ x ] "+r"(x));
    return x;
}