#pragma once

#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_mempool.h>

#include <boost/log/trivial.hpp>
#include <stdexcept>
#include <stdint.h>

#include "Definitions.hpp"
#include "PacketDissection/PacketInfo.hpp"
#include "PacketDissection/PacketInfoCreator.hpp"

#define ERROR_STR_INDEX_OUT_OF_BOUNDS                                          \
    "index is out of bounds of PacketContainer. The highest possible index "   \
    "can be accessed by get_total_number_of_packets()."

/**
 * This class can poll, send and hold packets.
 *
 * Think of this data type as a list of packets holding the packets themself
 * as well as meta information per packet. If you want to access a certain
 * packet (e.g. within a for function) you can use the methods provided with
 * a packet index.
 *
 * You can iterate either only over packets that were polled or over
 * all packets in the container.
 *    - Usually if you create a packet you also want to send it and do not
 * look at it anymore after the creation. So the only packets to look at are
 * those that where being polled. If this is the case it would be more
 * efficient to only iterate over polled packets.
 *    - The latter includes both packets polled and packets you
 * created yourselve with one of the corresponding methods. It could be
 * helpful in other situations.
 *
 * The maximum of packets to iterate over is therefore the
 * number of polled packets / the total number of packets which both can be
 * accessed each with the corresponding getter method. Remember that the
 * indices begin at 0 so the last element is at index (nb_packets - 1).
 *
 * To manipulate a packet / read data from it you have to get the PacketInfo
 * object at a cretain index with the corresponding method. Consult the
 * PacketInfo documentation for further information.
 *
 * NOTICE: It is possible that you get nullptr instead of a PacketInfo
 * pointer if you get a packet with one of the corresponding methods. Test
 * if this is the case (e.g. ptr == nullptr) before going ahead working with
 * the PacketInfo object.
 *
 * If you want to create a new packet do not construct a new PacketInfo
 * object. Use the method get_empty_packet instead. This is more efficient.
 *
 * An object of this class is initialized with the ID of the rx queue and the
 * tx queue of the thread that uses the object. Since corresponding [r|t]x
 * queues each on one port have the same ID it is only necessary to store the ID
 * of one rx queue and one tx queue.
 */
class PacketContainer {
  public:
    /**
     * @brief Construct a new Packet Container object
     *
     * @param pkt_handler
     * @param mbuf_pool
     * @param entrance_port
     * @param exit_port
     * @param rx_queue_number ID of the rx queues
     * @param tx_queue_number ID of the tx queues
     */
    inline PacketContainer(struct rte_mempool* mbuf_pool,
                           uint16_t entrance_port, uint16_t exit_port,
                           uint16_t rx_queue_number, uint16_t tx_queue_number)
        : _mempool(mbuf_pool), _entrance_port(entrance_port),
          _exit_port(exit_port), _nb_pkts_polled(0), _nb_pkts_total(0),
          _nb_pkts_dropped(0), _nb_pkt_arrays(1),
          _rx_queue_number(rx_queue_number), _tx_queue_number(tx_queue_number) {

        for (int i = 0; i < NUM_MBUF_ARRS; ++i) {
            _nb_mbufs_in_mbuf_arr[i] = 0;

            for (int j = 0; j < BURST_SIZE; ++j) {
                _mbuf_arrs[i][j] = nullptr;
                _pkt_info_arrs[i][j] = nullptr;
            }
        }
    }

    ~PacketContainer() { empty_container(); }

    /**
     * @brief Poll packets into the container
     *
     * The operation poll_packets does not guarantee that more than 0 packets
     * were polled. If there are no packets on the NIC port, nothing can be
     * polled. So you have to be able to check if packets actually were polled.
     * This could be important, e.g if you want to skip certain parts of your
     * program if no packets were polled. For the sake of efficiency you do not
     * do this with a getter that returns the state but with a pass-by-reference
     * variable that you pass to the poll_packets operation- After the execution
     * of poll_packets this variable holds the number of packets actually
     * polled.
     *
     * Also if you poll packets but did not send the packets currently being in
     * the PacketContainer they are going to be overwritten.
     *
     * @param[out] nb_pkts_polled after execution: holds number of packets
     * actually polled.
     */
    inline void poll_packets(uint16_t& nb_pkts_polled) {
        empty_container();

        _nb_pkts_polled = rte_eth_rx_burst(_entrance_port, _rx_queue_number,
                                           _mbuf_arrs[0], BURST_SIZE);

        _nb_pkts_total = _nb_pkts_polled;

        if (_nb_pkts_polled > 0) {
            _nb_pkt_arrays = 1;
            _nb_mbufs_in_mbuf_arr[0] = _nb_pkts_polled;
        }

        extract_header_info();

        // return
        nb_pkts_polled = _nb_pkts_polled;
    }

    /**
     * @brief Send packets from the container
     *
     * Sent packets are no longer available after being sent.
     * @return total number of packets send
     */
    inline uint send_packets() {
        uint sum_pkts_send = 0;
        if (likely(_nb_pkts_total > 0)) {
            if (_nb_pkts_dropped != 0) {
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
                         << " and this should have been sent: "
                         << nb_pkts_to_send << LOG_END;
#endif

                sum_pkts_send += nb_pkts_sent;
                // Free any unsent packets.
                if (unlikely(nb_pkts_sent < nb_pkts_to_send)) {
                    for (int j = nb_pkts_sent; j < nb_pkts_to_send; ++j) {
                        rte_pktmbuf_free(_mbuf_arrs[i][j]);
                    }
                }
            }
        }

        empty_container();
        return sum_pkts_send;
    }

    /**
     * @brief Drop packet.
     *
     * @param index Index of the packet to drop.
     */
    inline void drop_packet(int index) {
        if (index >= _nb_pkts_total) {
            throw std::runtime_error(ERROR_STR_INDEX_OUT_OF_BOUNDS);
        }

        int i = -1;
        int j = -1;
        calculate_array_coordinates_from_index(index, i, j);

        PacketInfoCreator::destroy_pkt_info(_pkt_info_arrs[i][j]);

        if (unlikely(_mbuf_arrs[i][j] != nullptr)) {
            rte_pktmbuf_free(_mbuf_arrs[i][j]);
        }

        _pkt_info_arrs[i][j] = nullptr;
        _mbuf_arrs[i][j] = nullptr;

        ++_nb_pkts_dropped;
    }

    /**
     * @brief Drop packet.
     *
     * @param index Index of the packet to drop.
     */
    inline void drop_packet(int index, PacketInfoIpv4Tcp* pkt_to_drop) {
        if (index >= _nb_pkts_total) {
            throw std::runtime_error(ERROR_STR_INDEX_OUT_OF_BOUNDS);
        }

        int i = -1;
        int j = -1;
        calculate_array_coordinates_from_index(index, i, j);

        delete pkt_to_drop;

        if (unlikely(_mbuf_arrs[i][j] != nullptr)) {
            rte_pktmbuf_free(_mbuf_arrs[i][j]);
        }

        _pkt_info_arrs[i][j] = nullptr;
        _mbuf_arrs[i][j] = nullptr;

        ++_nb_pkts_dropped;
    }

    /**
     * @brief Take the packet
     *
     * Return a PacketInfo object. The corresponding packet will be removed from
     * the PacketContainer so it is no longer in there and will not be sent. The
     * only reference available to the packet is the returned PacketInfo
     * pointer.
     *
     * Because the packet is removed from the container the responsibility for
     * dropping|sending the packet is up to you. Usually you would add this
     * packet to a packet container and sen dor drop the packet afterwards.
     *
     * If you want to just get the pointer to a PacketInfo object that sould
     * remain in the container do not use this method.
     *
     * @param index
     * @return PacketInfo*
     */
    inline PacketInfo* take_packet(int index) {
        if (index >= _nb_pkts_total) {
            throw std::runtime_error(ERROR_STR_INDEX_OUT_OF_BOUNDS);
        }

        int i, j;
        calculate_array_coordinates_from_index(index, i, j);
        PacketInfo* pkt_info = _pkt_info_arrs[i][j];
        _pkt_info_arrs[i][j] = nullptr;
        _mbuf_arrs[i][j] = nullptr;
        ++_nb_pkts_dropped;

        ++_nb_pkts_dropped;

        return pkt_info;
    }

    /**
     * @brief add Packet to Container which already exists but is stored
     * somewhere else.
     *
     * Add Packet to container which already exists but is stored
     * somewhere else. All values (payload and metadata) of the passed mbuf will
     * be adopted. These are set as
     * parameter of the function call.
     *
     * @param[in] pkt_container packet container pointer to the packet to be
     * added
     * @return index of the newly added packet
     */
    inline int add_packet(PacketInfo* pkt_info) {
        int i, j;
        calculate_array_coordinates_from_index(_nb_pkts_total, i, j);
        _mbuf_arrs[i][j] = pkt_info->get_mbuf();
        _pkt_info_arrs[i][j] = pkt_info;

        ++_nb_pkts_total;
        ++_nb_mbufs_in_mbuf_arr[i];

        if (unlikely(BURST_SIZE * _nb_pkt_arrays < _nb_pkts_total)) {
            ++_nb_pkt_arrays;
        }

        return _nb_pkts_total - 1;
    }

    /**
     * @brief Get the PacketInfo object at index
     *
     * Use this Method to get a packet and work with it (read, write
     * information). You do not have to add it to the PacketContainer again. The
     * packet is still in the Container you only get the reference.
     *
     * @param index
     * @return PacketInfo*
     */
    inline PacketInfo* get_packet_at_index(int index) {
        if (index >= _nb_pkts_total) {
            throw std::runtime_error(ERROR_STR_INDEX_OUT_OF_BOUNDS);
        }

        int i, j;
        calculate_array_coordinates_from_index(index, i, j);

        return _pkt_info_arrs[i][j];
    }

    /**
     * @brief Get new empty packets PacketInfo to specified packet type
     *
     * This method is to get you a pointer to an empty PacketInfo object of
     * specified type. This pointer is diguised as normal PacketInfo. You can
     * work with the PacketInfo object as you want then. You have to cast the
     * pointer to specified PacketInfo version to use specific IP and L4
     * protocol functions. You do not have to add it to the PacketContainer
     * again. The packet is still in the Container you only get the reference.
     * This packetInfo already has a mbuf.
     *
     * @param pkt_type specifies type of PacketInfo which should be created
     * @return PacketInfo*
     */
    inline PacketInfo* get_empty_packet(PacketType pkt_type) {
        int i, j;
        calculate_array_coordinates_from_index(_nb_pkts_total, i, j);

        ++_nb_pkts_total;
        ++_nb_mbufs_in_mbuf_arr[i];

        if (unlikely(BURST_SIZE * _nb_pkt_arrays < _nb_pkts_total)) {
            ++_nb_pkt_arrays;
        }
        _mbuf_arrs[i][j] = rte_pktmbuf_alloc(_mempool);
        _pkt_info_arrs[i][j] =
            PacketInfoCreator::create_pkt_info(_mbuf_arrs[i][j], pkt_type);
        return _pkt_info_arrs[i][j];
    }

    /**
     * @brief Get new empty packets PacketInfo to a IPv4TCP packet
     *
     * This method is to get you a pointer to an empty PacketInfoIpv4Tcp object.
     * This pointer is diguised as normal PacketInfo. You can work with the
     * PacketInfo object as you want then. You have to cast the pointer to
     * PacketInfoIpv4Tcp to use specific IPv4 and TCP functions. You do not have
     * to add it to the PacketContainer again. The packet is still in the
     * Container you only get the reference. This packetInfo already has a mbuf.
     *
     * @return PacketInfo*
     */
    inline PacketInfo* get_empty_packet() { return get_empty_packet(IPv4TCP); }

    /**
     * @brief Get the number of polled packets
     *
     * @return uint16_t
     */
    inline uint16_t get_number_of_polled_packets() { return _nb_pkts_polled; }

    /**
     * @brief Get the total number of packets in the container
     *
     * @return uint16_t
     */
    inline uint16_t get_total_number_of_packets() { return _nb_pkts_total; }

    /**
     * @brief Get the mempool
     *
     * @return rte_mempool*
     */
    inline rte_mempool* get_mempool() { return _mempool; }

#ifdef TEST
    uint16_t* get_nb_mbufs_in_mbuf_arr() { return _nb_mbufs_in_mbuf_arr; }

    int get_nb_pkts_dropped() { return _nb_pkts_dropped; }
#endif

  protected:
    struct rte_mempool* _mempool;

    uint16_t _rx_queue_number;
    uint16_t _tx_queue_number;

    /**
     * Port ID of the port the packets are to be polled
     * from
     */
    uint16_t _entrance_port;
    /**
     * Port ID of the port the packets are to be sent
     * to
     */
    uint16_t _exit_port;

    /**
     * Number of packets that where polled. Newly
     * created or added packets are not counted.
     */
    uint16_t _nb_pkts_polled;
    /**
     * Total number of mbufs their references are
     * stored in _mbuf_arrs. nullptrs in between valid mbuf pointers are
     * counted, too.
     */
    uint16_t _nb_pkts_total;

    /**
     * Number of Arrays that are stored in _mbuf_arrs
     */
    int _nb_pkt_arrays;

    /**
     * Total number of Packets dropped; is increased when a packet is removed
     * from the container by take_packet or drop_packet
     * \todo rename to _nb_pkts_removed
     */
    int _nb_pkts_dropped;

    /**
     * @brief Array of arrays of [rte_mbuf* |
     * PacketInfo*]
     *
     * A 2d-array is used instead of just a 1d-array to
     * be able to store multiple arrays with the fixed
     * size of BURST_SIZE. This is necessary if new
     * packets are created but the first array is full.
     * In this case a new array would be used to store
     * newly created packets. Otherwise only one
     * 1d-Array is used, the 2d-array-array would have
     * just one element then.
     *
     * Each PacketInfo object is assignet to one and
     * only one mbuf. PacketInfo at index "i" holds
     * information to mbuf at index "i".
     */
    /**@{*/
    struct rte_mbuf* _mbuf_arrs[NUM_MBUF_ARRS][BURST_SIZE];
    PacketInfo* _pkt_info_arrs[NUM_MBUF_ARRS][BURST_SIZE];
    /**@}*/

    /**
     * Number of mbufs in each sub-array of _mbuf_arrs. nullptrs in between
     * valid mbuf pointers are counted, too.
     */
    uint16_t _nb_mbufs_in_mbuf_arr[NUM_MBUF_ARRS];

#ifdef TEST
  public:
#endif
    /**
     * @brief Reorder the mbuf arrays; meaning remove nullptrs. So no nullptr
     * will be given to the send_packets_to_port method of the
     * NetworkPacketHandler.
     */
    inline void reorder_mbuf_arrays() {
        if (likely(_nb_pkts_total > 0)) {
            for (int i = 0; i < _nb_pkt_arrays; ++i) {

                // go through mbufs, reorder
                int len = _nb_mbufs_in_mbuf_arr[i];
                int nb_elements_to_skip = 0;

                for (int j = 0; j < len; ++j) {
                    if (_mbuf_arrs[i][j] == nullptr) {
                        ++nb_elements_to_skip;
                    } else if (nb_elements_to_skip > 0) {
                        _mbuf_arrs[i][j - nb_elements_to_skip] =
                            _mbuf_arrs[i][j];
                        _mbuf_arrs[i][j] = nullptr;
                    }
                }

                _nb_mbufs_in_mbuf_arr[i] -= nb_elements_to_skip;
            }
        }
    }
#ifdef TEST
  protected:
#endif

    /**
     * @brief Calculate the "i" and "j" coordinates for
     * a 2d array from a 1d integer "index".
     *
     * @param[in] index
     * @param[out] i
     * @param[out] j
     */
    inline void calculate_array_coordinates_from_index(int index, int& i,
                                                       int& j) {
        i = (index - (index % BURST_SIZE)) / BURST_SIZE;
        j = index % BURST_SIZE;
    }

    /**
     * @brief Does the what
     * calculate_array_coordinates_from_index does but
     * in the other direction.
     *
     * Beware: The order of the arguments is important!
     *
     * @param[in] i first dimesnion index
     * @param[in] j second dimension index
     * @param[out] index
     */
    inline void calculate_index_from_array_coordinates(int i, int j,
                                                       int& index) {
        index = i * BURST_SIZE + j;
    }

    /**
     * @brief Set the _state of the container to EMPTY
     * and assign variables to 0.
     */
    inline void empty_container() {
        int nb_pkts_remaining = _nb_pkts_total;

        for (int i = 0; i < _nb_pkt_arrays; ++i) {
            int len = _nb_mbufs_in_mbuf_arr[i];

            for (int j = 0; j < len; ++j) {
                if (_pkt_info_arrs[i][j] != nullptr) {
                    PacketInfoCreator::destroy_pkt_info(_pkt_info_arrs[i][j]);
                    _pkt_info_arrs[i][j] = nullptr;
                }

                _mbuf_arrs[i][j] = nullptr;
            }

            _nb_mbufs_in_mbuf_arr[i] = 0;
        }

        _nb_pkts_polled = 0;
        _nb_pkts_total = 0;
        _nb_pkts_dropped = 0;
        _nb_pkt_arrays = 0;
    }

    /**
     * @brief creates a PacketInfo for every packet
     * in PacketContainer and saves them with
     * corresponding index in PacketInfo Array
     *
     */
    inline void extract_header_info() {
        for (int i = 0; i < _nb_pkts_polled; ++i) {
            _pkt_info_arrs[0][i] =
                fill_info(_mbuf_arrs[0][i], _pkt_info_arrs[0][i]);
        }
    }

    /**
     * @brief creates PacketInfo for given mbuf
     *
     * @param[in] mbuf dpdk abstraction of a packet
     * for which a PacketInfo should be created
     * @param[out] pkt_inf newly created PacketInfo
     */
    inline PacketInfo* fill_info(rte_mbuf* mbuf, PacketInfo* pkt_inf) {
        pkt_inf = PacketInfoCreator::create_pkt_info(mbuf);
        return pkt_inf;
    }
};