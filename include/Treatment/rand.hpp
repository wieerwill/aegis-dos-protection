#pragma once

/**
 * @file rand.hpp
 * @author Felix, Fabienne
 * @brief This header includes only one method which returns a random 64bit
 * unsigned integer
 * @version 0.1
 * @date 2021-07-06
 *
 * @copyright Copyright (c) 2021
 *
 */

/**
 * @brief Rand itself provides a method to get a random number. This number is
 * used in Treatment as cookie_secret
 *
 */
class Rand {

  public:
    /**
     * @brief Get a random 64bit unsigned integer
     *
     * @return u_int64_t
     */
    static u_int64_t get_random_64bit_value() {
        u_int64_t random64BitNumber = 0;

        u_int64_t value1 = (uint16_t)std::rand();
        value1 = (value1 << 48);
        random64BitNumber |= value1;

        u_int64_t value2 = (uint16_t)rand();
        value2 = (value2 << 32);
        random64BitNumber |= value2;

        u_int64_t value3 = (uint16_t)rand();
        random64BitNumber |= value3;

        return random64BitNumber;
    }
};