#pragma once

#include <cstdlib>
#include <stdint.h>

/**
 * @brief Generate random unassigned 16, 32 and 64 bit integer using
 * XorShift algorithm and rand() for generating a seed.
 */
class RandomNumberGenerator {
  public:
    uint16_t _seed_x16;
    uint32_t _seed_x32;
    uint64_t _seed_x64;

    /**
     * @brief Construct a new Random Number Generator object
     */
    inline RandomNumberGenerator()
        : _seed_x16(rand()), _seed_x32(rand()), _seed_x64(rand()) {}

    /**
     * @brief generates a 16 bit unsigned integer between lower limit and
     * upper limit (1024 and 49151 for valid port numbers)
     *
     * @return uint16_t
     */
    inline uint16_t gen_rdm_16_bit_in_interval(uint16_t _lower_limit,
                                               uint16_t _upper_limit) {
        _seed_x16 ^= _seed_x16 << 7;
        _seed_x16 ^= _seed_x16 >> 9;
        _seed_x16 ^= _seed_x16 << 8;
        // this method returns a valid port number
        // range should be: 1024 to 49152
        return _seed_x16 % (_upper_limit - _lower_limit) + _lower_limit;
    }

    /**
     * @brief generates 16 bit unsigned integer
     *
     * @return uint16_t
     */
    inline uint16_t gen_rdm_16_bit() {
        _seed_x16 ^= _seed_x16 << 7;
        _seed_x16 ^= _seed_x16 >> 9;
        _seed_x16 ^= _seed_x16 << 8;
        return _seed_x16;
    }

    /**
     * @brief generates 32 bit unsigned integer
     *
     * @return uint32_t
     */
    inline uint32_t gen_rdm_32_bit() {
        _seed_x32 ^= _seed_x32 << 14;
        _seed_x32 ^= _seed_x32 >> 13;
        _seed_x32 ^= _seed_x32 << 15;
        return _seed_x32;
    }

    /**
     * @brief generates 64 bit unsigned integer
     *
     * @return uint64_t
     */
    inline uint64_t gen_rdm_64_bit() {
        _seed_x64 ^= _seed_x64 << 14;
        _seed_x64 ^= _seed_x64 >> 23;
        _seed_x64 ^= _seed_x64 << 33;
        return _seed_x64;
    }
};