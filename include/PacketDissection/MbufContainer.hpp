#pragma once

#include <rte_ethdev.h>
#include <rte_mbuf.h>

#include <stdexcept>

#include "Definitions.hpp"

#define ERROR_STR_MBUF_CONTAINER_INDEX_OUT_OF_BOUNDS                           \
    "Index is out of bounds of MbufContainer."

/**
 * @brief like the PacketContainer but it only holds mbufs plus some minor
 * performance improvements
 *
 * \todo Has to be documented with more details
 *
 */
class MbufContainer {
  protected:
    uint16_t _nb_mbufs_in_container;

    rte_mbuf* _mbuf_arr[BURST_SIZE];

    rte_mempool* _mempool;

  public:
    inline MbufContainer(rte_mempool* mempool)
        : _mempool(mempool), _nb_mbufs_in_container(0) {}

    /**
     * @brief Get the mbuf at index
     *
     * @param index
     * @return rte_mbuf*
     */
    inline rte_mbuf* get_mbuf_at_index(int index) {
        check_index(index);
        return _mbuf_arr[index];
    }

    /**
     * @brief Get the number of mbufs
     *
     * @return uint16_t
     */
    inline uint16_t get_number_of_mbufs() { return _nb_mbufs_in_container; }

    inline void check_index(int index) {
        if (unlikely(index >= _nb_mbufs_in_container || index < 0)) {
            throw std::runtime_error(
                ERROR_STR_MBUF_CONTAINER_INDEX_OUT_OF_BOUNDS);
        }
    }
};