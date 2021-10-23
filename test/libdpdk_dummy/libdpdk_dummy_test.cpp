#include <catch2/catch.hpp>
#include <cstdio>
#include <iostream>

#include <rte_mbuf.h>
#include <rte_mempool.h>

TEST_CASE("rte_mbuf", "[]") {
    struct rte_mbuf* mbuf;
    struct rte_mempool* mempool = nullptr;

    mbuf = rte_pktmbuf_alloc(mempool);
    CHECK(mbuf != nullptr);
}