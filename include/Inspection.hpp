/**
 * @file Inspection.hpp
 * @author Robert
 * @brief the Inspection get packets polled by the PacketContainer and evaluates
 * each packets legitimation. Invalid packets will be dropped while valid
 * packets are forwarded to the Treatment.
 * @version 0.5
 * @date 2021-06-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#pragma once
#include "Configurator.hpp"
#include "PacketDissection/PacketContainer.hpp"
#include <rte_cycles.h>
#include <stdint.h>

/**
 * @brief the Inspection evaluates PacketContainers and forwards valid packets
 * while illegimite packets are destroyed
 *
 */
class Inspection {
  public:
    /**
     * @brief Construct a new Inspection object with default arguments from
     * configuration file
     *
     */
    inline Inspection()
        : _UDP_flood_weight(
              Configurator::instance()->get_config_as_unsigned_int(
                  "UDP_flood_weight")),
          _TCP_flood_weight(
              Configurator::instance()->get_config_as_unsigned_int(
                  "TCP_flood_weight")),
          _ICMP_flood_weight(
              Configurator::instance()->get_config_as_unsigned_int(
                  "ICMP_flood_weight")),
          _SYNFIN_weight(Configurator::instance()->get_config_as_unsigned_int(
              "SYNFIN_weight")),
          _SMALLWINDOW_weight(
              Configurator::instance()->get_config_as_unsigned_int(
                  "SMALLWINDOW_weight")),
          _threshold_UDP(Configurator::instance()->get_config_as_unsigned_int(
              "threshold_UDP")),
          _threshold_TCP(Configurator::instance()->get_config_as_unsigned_int(
              "threshold_TCP")),
          _threshold_ICMP(Configurator::instance()->get_config_as_unsigned_int(
              "threshold_ICMP")),
          _current_threshold_UDP(
              Configurator::instance()->get_config_as_unsigned_int(
                  "threshold_UDP")),
          _current_threshold_TCP(
              Configurator::instance()->get_config_as_unsigned_int(
                  "threshold_TCP")),
          _current_threshold_ICMP(
              Configurator::instance()->get_config_as_unsigned_int(
                  "threshold_ICMP")),
          _min_window_size(Configurator::instance()->get_config_as_unsigned_int(
              "min_window_size")),
          _attack_level(0), _packetrate_UDP(0), _packetrate_TCP(0),
          _packetrate_ICMP(0), _clock_frequence(rte_get_tsc_hz()) {}

    /**
     * @brief update the current stats and send them to global statistic
     *
     * @param udp_packets
     * @param tcp_packets
     * @param icmp_packets
     * @param UDP_Floods
     * @param TCP_Floods
     * @param ICMP_Floods
     * @param SYN_FINs
     * @param Small_Windows
     * @param duration
     */
    inline void update_stats(uint16_t udp_packets, uint16_t tcp_packets,
                             uint16_t icmp_packets, uint16_t UDP_Floods,
                             uint16_t TCP_Floods, uint16_t ICMP_Floods,
                             uint16_t SYN_FINs, uint16_t Small_Windows,
                             uint64_t duration) {
        duration = duration > 1 ? duration : 1;
        duration = duration * _clock_frequence;
        // calculate new packetrate based on relative duration
        _packetrate_UDP = udp_packets / duration;
        _packetrate_TCP = tcp_packets / duration;
        _packetrate_ICMP = icmp_packets / duration;

        // calculate attack level based on diverent counted attacks and their
        // weight on ddos
        /// \todo (can) adapting weights to most occuring attacks
        _attack_level =
            UDP_Floods * _UDP_flood_weight + TCP_Floods * _TCP_flood_weight +
            ICMP_Floods * _ICMP_flood_weight + SYN_FINs * _SYNFIN_weight +
            Small_Windows * _SMALLWINDOW_weight;

        // calculate new threshold
        _current_threshold_UDP = _threshold_UDP - (1 / _threshold_UDP) *
                                                      _attack_level *
                                                      _attack_level;
        _current_threshold_TCP = _threshold_TCP - (1 / _threshold_TCP) *
                                                      _attack_level *
                                                      _attack_level;
        _current_threshold_ICMP = _threshold_ICMP - (1 / _threshold_ICMP) *
                                                        _attack_level *
                                                        _attack_level;

        /// \todo send to global statistic
        /* Stats update_statistics(
            attacks = UDP_Floods + TCP_Floods + ICMP_Floods + SYN_FINs +
           Small_Windows;
            // bytes
            // dropped
            packets = udp_packets + tcp_packets + icmp_packets;
            work_time = duration;
        */
    }

    /**
     * @brief get packet container reference and run analysis on each packet
     * included in the PacketContainer
     *
     * @param pkt_container PacketContainer with certain amount of packets
     *
     * \startuml
     *  title analyzePacket(PacketContainer)
     *  start
     *  :create temporary counters;
     *  :initiate packetIndex at 0;
     *  while(PacketContainer size > current packetIndex)
     *    :extract PacketInfo from PacketContainer at packetIndex;
     *    if (is UDP Protocoll) then (yes)
     *      :increase UDP counter;
     *      if(UDP Threshold met) then (yes)
     *        #FF0000:detected UDP Flood;
     *        :increase attack counter;
     *        :drop packet;
     *      else (no)
     *      endif
     *    elseif (is TCP Protocoll) then (yes)
     *      :increase TCP counter;
     *      if(TCP Threshold met) then (yes)
     *        #FF0000:detected TCP Flood;
     *        :increase attack counter;
     *        :drop packet;
     *      else (no)
     *      endif
     *      if(WindowSize too small) then (yes)
     *        #FF0000:detected Small/Zero Window;
     *        :increase attack counter;
     *        :drop packet;
     *      else (no)
     *      endif
     *      if(Flag Pattern forbidden) then (yes)
     *        #FF0000:detected SYN-FIN(-ACK);
     *        :increase attack counter;
     *        :drop packet;
     *      else (no)
     *      endif
     *    elseif (is ICMP Protocoll) then (yes)
     *      :increase ICMP counter;
     *      if(ICMP Threshold met) then (yes)
     *        #FF0000:detected ICMP Flood;
     *        :increase attack counter;
     *        :drop packet;
     *      else (no)
     *      endif
     *    else (no protocoll of interest)
     *    endif
     *  :increase packetIndex;
     *  endwhile
     *  :update local threshold with temporary counters;
     *  :send temporary counters to global statistic;
     *  end
     * \enduml
     */
    inline void analyze_container(PacketContainer* pkt_container) {
        uint16_t temp_udp_packets = 0;
        uint16_t temp_tcp_packets = 0;
        uint16_t temp_icmp_packets = 0;

        uint16_t temp_attack_UDP_Flood = 0;
        uint16_t temp_attack_TCP_Flood = 0;
        uint16_t temp_attack_ICMP_Flood = 0;
        uint16_t temp_attack_SYN_FIN = 0;
        uint16_t temp_attack_Small_Window = 0;

        uint64_t startTime = rte_get_tsc_cycles();

        for (int index = 0;
             index < pkt_container->get_number_of_polled_packets(); ++index) {
            PacketInfo* packetInfo = pkt_container->get_packet_at_index(index);
            if (packetInfo != nullptr) {
                switch (packetInfo->get_type()) {
                case IPv4UDP: {
                    ++temp_udp_packets;
                    if (temp_udp_packets > _current_threshold_UDP) {
                        // alt: if(_current_threshold_UDP > temp_udp_packets) {
                        ++temp_attack_UDP_Flood;
                        // UDP Flood
                        /// \todo (can) create patchmap and allow last recent
                        /// established connections to connect further matching
                        /// srcIp & dstIp
                        pkt_container->drop_packet(index);
                        break;
                    }
                    // forward packet
                    break;
                }
                case IPv4TCP: {
                    PacketInfoIpv4Tcp* TCP4packetInfo =
                        static_cast<PacketInfoIpv4Tcp*>(packetInfo);
                    ++temp_tcp_packets;
                    if (temp_tcp_packets > _current_threshold_TCP) {
                        // TCP Flood
                        /// \todo (can) create patchmap and allow last recent
                        /// established connections to connect further matching
                        /// srcIp & dstIp
                        ++temp_attack_TCP_Flood;
                        pkt_container->drop_packet(index);
                        break;
                    }
                    if (TCP4packetInfo->get_window_size() < _min_window_size) {
                        // Zero/Small Window
                        ++temp_attack_Small_Window;
                        pkt_container->drop_packet(index);
                        break;
                    }
                    if ((uint8_t(TCP4packetInfo->get_flags()) & 0b00000011) ==
                        0b00000011) {
                        // flags in reverse format
                        // FIN,SYN,RST,PSH,ACK,URG,ECE,CWR SYN-FIN/SYN-FIN-ACK
                        ++temp_attack_SYN_FIN;
                        pkt_container->drop_packet(index);
                        break;
                    }
                    // forward packet
                    break;
                }
                case IPv4ICMP: {
                    ++temp_icmp_packets;
                    if (temp_icmp_packets > _current_threshold_ICMP) {
                        // ICMP Flood
                        ++temp_attack_ICMP_Flood;
                        pkt_container->drop_packet(index);
                        break;
                    }
                    // forward packet
                    break;
                }
                default: { // not identifyable or not of interest
                    // forward packet
                    break;
                }
                }
            }
        }

        uint64_t endTime = rte_get_tsc_cycles();

        update_stats(temp_udp_packets, temp_tcp_packets, temp_icmp_packets,
                     temp_attack_UDP_Flood, temp_attack_TCP_Flood,
                     temp_attack_ICMP_Flood, temp_attack_SYN_FIN,
                     temp_attack_Small_Window, (uint64_t)endTime - startTime);
        return;
    }

    uint16_t get_UDP_packet_rate() { return _packetrate_UDP; };
    uint16_t get_TCP_packet_rate() { return _packetrate_TCP; };
    uint16_t get_ICMP_packet_rate() { return _packetrate_ICMP; };
    uint16_t get_UDP_threshold() { return _current_threshold_UDP; };
    uint16_t get_TCP_threshold() { return _current_threshold_TCP; };
    uint16_t get_ICMP_threshold() { return _current_threshold_ICMP; };
    uint16_t get_attack_level() { return _attack_level; };

  private:
    /// weight of UDP Flood attacks
    uint8_t _UDP_flood_weight;
    /// weight of TCP Flood attacks
    uint8_t _TCP_flood_weight;
    /// weight of ICMP Flood attacks
    uint8_t _ICMP_flood_weight;
    /// weight of SYN-FIN and SYN_FIN_ACK attacks
    uint8_t _SYNFIN_weight;
    /// weight of Zero/Small Window attacks
    uint8_t _SMALLWINDOW_weight;
    /// absolute threshold of udp packets
    uint16_t _threshold_UDP;
    /// absolute threshold of tcp packets
    uint16_t _threshold_TCP;
    /// absolute threshold of icmp packets
    uint16_t _threshold_ICMP;
    /// current threshold of udp packets
    uint16_t _current_threshold_UDP;
    /// current threshold of tcp packets
    uint16_t _current_threshold_TCP;
    /// current threshold of icmp packets
    uint16_t _current_threshold_ICMP;
    /// min window size for no-small-window
    uint16_t _min_window_size;
    /// current attack level
    uint16_t _attack_level;
    /// current packetrate of udp packets
    uint64_t _packetrate_UDP;
    /// current packetrate of tcp packets
    uint64_t _packetrate_TCP;
    /// current packetrate of icmp packets
    uint64_t _packetrate_ICMP;
    /// TSC frequency for this lcore
    uint64_t _clock_frequence;
};
