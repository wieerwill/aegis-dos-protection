#pragma once

#include <rte_ethdev.h>
#include <rte_mbuf.h>

#include "PacketDissection/MbufContainer.hpp"

/**
 * @brief for transmitted mbufs.
 *
 */
class MbufContainerTransmitting : public MbufContainer {
  private:
    uint16_t _tx_port;
    uint16_t _tx_queue;

  public:
    inline MbufContainerTransmitting(rte_mempool* mempool, uint16_t tx_port,
                                     uint16_t tx_queue)
        : MbufContainer(mempool), _tx_port(tx_port), _tx_queue(tx_queue) {}

    /**
     * @brief add mbuf to container
     *
     * This method may delete the mbuf given if the container is full
     *
     * @param mbuf
     */
    inline void add_mbuf(rte_mbuf* mbuf) {
        if (likely(_nb_mbufs_in_container < BURST_SIZE)) {
            _mbuf_arr[_nb_mbufs_in_container] = mbuf;
            ++_nb_mbufs_in_container;
        } else {
            rte_pktmbuf_free(mbuf);
        }
    }

    /**
     * @brief create new mbuf in the container and return the pointer to it
     *
     * @return rte_mbuf*:
     *  - normally: a newly created mbuf is returned
     *  - if container is full and no mbuf can be added anymore: nullptr
     */
    inline rte_mbuf* get_empty_mbuf() {
        if (likely(_nb_mbufs_in_container < BURST_SIZE)) {
            _mbuf_arr[_nb_mbufs_in_container] = rte_pktmbuf_alloc(_mempool);
            return _mbuf_arr[_nb_mbufs_in_container++];
        } else {
            return nullptr;
        }
    }

    /**
     * @brief send every mbuf contained in the container
     *
     */
    inline void send_mbufs() {
        if (likely(_nb_mbufs_in_container > 0)) {

            // send mbufs
            uint16_t nb_mbufs_sent = rte_eth_tx_burst(
                _tx_port, _tx_queue, _mbuf_arr, _nb_mbufs_in_container);

            // Free any unsent packets.
            if (unlikely(nb_mbufs_sent < _nb_mbufs_in_container)) {
                for (int i = nb_mbufs_sent; i < _nb_mbufs_in_container; ++i) {
                    rte_pktmbuf_free(_mbuf_arr[i]);
                }
            }

            _nb_mbufs_in_container = 0;
        }
    }
};