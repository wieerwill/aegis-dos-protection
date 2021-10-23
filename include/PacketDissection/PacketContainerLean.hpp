#pragma once

#include "PacketDissection/PacketContainer.hpp"

/**
 * @brief This class holds the same functions like the PacketContainer except it
 * only holds mbufs and no PacketInfo
 *
 * only send/reorder etc polled packets. Manually created packets are never
 * dropped.
 */
class PacketContainerLean : protected PacketContainer {
  public:
    inline PacketContainerLean(struct rte_mempool* mbuf_pool,
                               uint16_t entrance_port, uint16_t exit_port,
                               uint16_t rx_queue_number,
                               uint16_t tx_queue_number)
        : PacketContainer(mbuf_pool, entrance_port, exit_port, rx_queue_number,
                          tx_queue_number),
          _nb_polled_pkts_dropped(0) {}

    inline ~PacketContainerLean() { empty_container_lean(); }

    inline void poll_mbufs(uint16_t& nb_mbufs_polled) {
        empty_container_lean();

        _nb_pkts_polled = rte_eth_rx_burst(_entrance_port, _rx_queue_number,
                                           _mbuf_arrs[0], BURST_SIZE);

        _nb_pkts_total = _nb_pkts_polled;

        if (likely(_nb_pkts_polled > 0)) {
            _nb_pkt_arrays = 1;
            _nb_mbufs_in_mbuf_arr[0] = _nb_pkts_polled;
        }

        // return
        nb_mbufs_polled = _nb_pkts_polled;
    }

    inline uint send_mbufs() {
        uint sum_mbufs_send = 0;
        if (likely(_nb_pkts_total > 0)) {
            if (unlikely(_nb_polled_pkts_dropped != 0)) {
                reorder_mbuf_arrays();
            }

            for (int i = 0; i < _nb_pkt_arrays; ++i) {
                uint16_t nb_pkts_to_send = _nb_mbufs_in_mbuf_arr[i];

                // send mbufs
                uint16_t nb_pkts_sent =
                    rte_eth_tx_burst(_exit_port, _tx_queue_number,
                                     _mbuf_arrs[i], nb_pkts_to_send);

#ifdef LOG_PKTS_SENT
                LOG_INFO << "Number of packets sent: " << nb_pkts_sent
                         << LOG_END;
#endif

                sum_mbufs_send += nb_pkts_sent;
                // Free any unsent packets.
                if (unlikely(nb_pkts_sent < nb_pkts_to_send)) {
                    for (int j = nb_pkts_sent; j < nb_pkts_to_send; ++j) {
                        rte_pktmbuf_free(_mbuf_arrs[i][j]);
                    }
                }
            }
        }

        empty_container_lean();
        return sum_mbufs_send;
    }

    inline void drop_polled_mbuf(int index) {
        if (index >= _nb_pkts_polled) {
            throw std::runtime_error(
                ERROR_STR_INDEX_OUT_OF_BOUNDS
                " No... actually it is _nb_pkts_polled that is exceeded");
        }

        int i = -1;
        int j = -1;
        calculate_array_coordinates_from_index(index, i, j);

        if (unlikely(_mbuf_arrs[i][j] != nullptr)) {
            rte_pktmbuf_free(_mbuf_arrs[i][j]);
        }

        _pkt_info_arrs[i][j] = nullptr;
        _mbuf_arrs[i][j] = nullptr;

        ++_nb_polled_pkts_dropped;
    }

    inline rte_mbuf* get_empty_mbuf() {
        int i, j;
        calculate_array_coordinates_from_index(_nb_pkts_total, i, j);

        ++_nb_pkts_total;
        ++_nb_mbufs_in_mbuf_arr[i];

        if (unlikely(BURST_SIZE * _nb_pkt_arrays < _nb_pkts_total)) {
            ++_nb_pkt_arrays;
        }
        _mbuf_arrs[i][j] = rte_pktmbuf_alloc(_mempool);

        return _mbuf_arrs[i][j];
    }

    inline int add_mbuf(rte_mbuf* mbuf) {
        int i = -1;
        int j = -1;
        calculate_array_coordinates_from_index(_nb_pkts_total, i, j);
        _mbuf_arrs[i][j] = mbuf;

        ++_nb_pkts_total;
        ++_nb_mbufs_in_mbuf_arr[i];

        if (unlikely(BURST_SIZE * _nb_pkt_arrays < _nb_pkts_total)) {
            ++_nb_pkt_arrays;
        }

        return _nb_pkts_total - 1;
    }

    inline uint16_t get_number_of_polled_mbufs() { return _nb_pkts_polled; }

    inline rte_mbuf* get_mbuf_at_index(int index) {
        if (unlikely(index >= _nb_pkts_total)) {
            throw std::runtime_error(ERROR_STR_INDEX_OUT_OF_BOUNDS);
        }

        int i = -1;
        int j = -1;
        calculate_array_coordinates_from_index(index, i, j);

        return _mbuf_arrs[i][j];
    }

    inline rte_mempool* get_mempool_lean() { return get_mempool(); }

  private:
    int _nb_polled_pkts_dropped;

    inline void empty_container_lean() {

        empty_mbuf_arr();

        _nb_pkts_polled = 0;
        _nb_pkts_total = 0;
        _nb_pkts_dropped = 0;
        _nb_polled_pkts_dropped = 0;
        _nb_pkt_arrays = 0;
    }

    inline void empty_mbuf_arr() {
        for (int i = 0; i < _nb_pkt_arrays; ++i) {
            int len = _nb_mbufs_in_mbuf_arr[i];

            for (int j = 0; j < len; ++j) {
                _mbuf_arrs[i][j] = nullptr;
            }

            _nb_mbufs_in_mbuf_arr[i] = 0;
        }
    }
};