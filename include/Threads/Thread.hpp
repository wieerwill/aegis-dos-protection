/**
 * @file Thread.hpp
 * @author Jakob
 * @brief
 * @version 0.1
 * @date 2021-07-12
 *
 * @copyright Copyright (c) 2021
 *
 */

#pragma once

#include <stdint.h>

class Thread {
  public:
    /**
     * @brief Construct a new Thread object
     */
    inline Thread() : _quit(false), _running(true) {}

    /**
     * @brief quit the thread
     */
    inline void quit() { _quit = true; }

    inline bool is_running() { return _running; }

  protected:
    bool _quit;
    bool _running;
    uint64_t _cycles_old;
};