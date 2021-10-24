/**
 * @file DebugHelper.hpp
 * @author Tobias
 * @brief 
 * @version 0.1
 * @date 2021-07-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <iostream>

class DebugHelper{

    public:
        static void hex_dump_human_readable (const char *desc, const void *addr, int len);
        static void hex_dump_raw(const void *addr, int len);
};

