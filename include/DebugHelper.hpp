#pragma once

#include <iostream>

class DebugHelper{

    public:
        static void hex_dump_human_readable (const char *desc, const void *addr, int len);
        static void hex_dump_raw(const void *addr, int len);
};

