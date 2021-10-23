#pragma once

#include <rte_ethdev.h>
#include <rte_mbuf.h>

#include "Definitions.hpp"
#include "PacketDissection/MbufContainer.hpp"

/**
 * @brief for received mbufs. you poll them, read them and finally... forget
 * them
 *
 */
class MbufContainerReceiving : public MbufContainer {
  private:
    uint16_t _rx_port;
    uint16_t _rx_queue;

  public:
    inline MbufContainerReceiving(rte_mempool* mempool, uint16_t rx_port,
                                  uint16_t rx_queue)
        : MbufContainer(mempool), _rx_port(rx_port), _rx_queue(rx_queue) {}

    /**
     * @brief poll mbufs
     *
     * @return uint16_t
     */
    inline uint16_t poll_mbufs() {

        _nb_mbufs_in_container =
            rte_eth_rx_burst(_rx_port, _rx_queue, _mbuf_arr, BURST_SIZE);

        return _nb_mbufs_in_container;
    }
};