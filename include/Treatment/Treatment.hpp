#pragma once

#include "PacketDissection/MbufContainer.hpp"
#include "PacketDissection/MbufContainerReceiving.hpp"
#include "PacketDissection/MbufContainerTransmitting.hpp"
#include "PacketDissection/PacketContainer.hpp"
#include "PacketDissection/PacketInfo.hpp"
#include "PacketDissection/PacketInfoIpv4Tcp.hpp"
#include "rand.hpp"
#include "xxh3.hpp"
#include "xxhash.hpp"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <list>
#include <sparsehash/dense_hash_map>

/**
 * @file Treatment.h
 * @author Fabienne, Felix, Tim
 * @brief This header file includes the class structure, structs and needed
 * includes
 * @version 0.1
 * @date 2021-06-10
 *
 * @copyright Copyright (c) 2021
 *
 */

enum _flag_enum {
    SYN = 0b00000010,
    RST = 0b00000100,
    FIN = 0b00000001,
    ACK = 0b00010000,
    FINACK = 0b00010001,
    SYNACK = 0b00010010,
    SYNFIN = 0b00000011
};

/**
 * @brief Data struct used as the key in the _densemap.
 *
 * This Data struct is used as the key in the densemap. The parameters uniquely
 * identify each connection, and are therefore satisfying the requirements of
 * being a key in the densemap.
 * @param _extip Stores the IP Address of the external host
 * @param _intip Stores the IP Address of the internal host
 * @param _extport Stores the Port Address of the external host
 * @param _intport Stores the Port Address of the internal host
 *
 */
class Data {
  public:
    u_int32_t _extip;
    u_int32_t _intip;
    u_int16_t _extport;
    u_int16_t _intport;
    /**
     * @brief Construct a new Data object from scratch
     *
     */
    Data()
        : _extip(0)
        , _intip(0)
        , _extport(0)
        , _intport(0) {}
    /**
     * @brief Construct a new Data object
     *
     * @param _extip IP Address of external system
     * @param _intip IP Address of internal system
     * @param _extport Port of external system
     * @param _intport Port of internal system
     */
    Data(u_int32_t _extip, u_int32_t _intip, u_int16_t _extport,
         u_int16_t _intport)
        : _extip(_extip)
        , _intip(_intip)
        , _extport(_extport)
        , _intport(_intport) {}

    /**
     * @brief Redefinition of operator==
     * Needed in order to satisfy the external implementation of the used
     * hashfunction in combination with googles densemap
     *
     * @param d1
     * @return true
     * @return false
     */
    bool operator==(const Data& d1) const {
        return d1._extip == _extip && d1._intip == _intip &&
               d1._intport == _intport && d1._extport == _extport;
    }
};

/**
 *
 * @brief Info Struct used to encode the _offset and wether a finack has been
 * seen
 *
 * @param _offset Stores the difference between Sequence- and ACK-Numbers
 * @param _finseen Stores if a fin has already been seen
 * @param _pkt_inf Stores the PacketInfo for an ACK from connection
 * establishment
 */
class Info {

  public:
    int _offset;
    bool _finseen_to_inside;
    bool _finseen_to_outside;
    bool _ack_to_inside_expected;
    bool _ack_to_outside_expected;
    std::list<PacketInfoIpv4Tcp*> _pkt_inf_list;
    /**
     * @brief Construct a new Info object from scratch
     *
     */
    Info()
        : _offset(0)
        , _finseen_to_inside(false)
        , _finseen_to_outside(false)
        , _ack_to_inside_expected(false)
        , _ack_to_outside_expected(false)
        , _pkt_inf_list() {}
    /**
     * @brief Construct a new Info object
     * @param _offset Stores the difference between Sequence- and ACK-Numbers
     * @param _finseen  Stores if a fin has already been seen
     * @param _pkt_inf Stores the PacketInfo for an ACK from connection
     * establishment
     */
    Info(int offset, bool finseen_to_inside, bool finseen_to_outside,
         bool ack_to_inside, bool ack_to_outside, PacketInfoIpv4Tcp* pkt_inf)
        : _offset(offset)
        , _finseen_to_inside(finseen_to_inside)
        , _finseen_to_outside(finseen_to_outside)
        , _ack_to_inside_expected(ack_to_inside)
        , _ack_to_outside_expected(ack_to_outside) {
        if (pkt_inf != nullptr) {
            _pkt_inf_list.push_back(pkt_inf);
        }
    }
};

/**
 * @brief TreatmentHash manages the calculation of the hashvalue over a Data
 * Struct and is used in the _densemap
 *
 */
class TreatmentHash {
  public:
    /**
     * @brief Redefinition of the operator() used in the _densemap and _ackmap
     *
     * @param d Data Struct with encodings of internal and external IPs and
     * ports, which uniquely identify a connection
     * @return size_t Hashvalue used as the key in both _dense- and _ackmap
     */
    size_t operator()(const Data& d) const { // C++ call by reference
        return XXH3_64bits_withSeed(
            &d._extip, sizeof(d._extip),
            XXH3_64bits_withSeed(
                &d._intip, sizeof(d._intip),
                XXH3_64bits_withSeed(
                    &d._extport, sizeof(d._extport),
                    XXH3_64bits(&d._intport, sizeof(d._intport)))));
    }
};

/**
 * @brief Treatment class, containing all functionality of the Treatment.
 *
 * The Treatment itself provides an implementation of SYN-cookies, combined with
 * the functionalities of a TCP-Proxy, in order to migitate SYN-Floods. Other
 * attacks just as SYN-FIN, SYN-FIN-ACK or the UDP-Flood are already migitated
 * in the analyzer, as the analyzer already contains all the information to do
 * this. Thus function calls are reduced, ultimatively resulting in a better
 * performance.
 *
 */
class Treatment {
  public:
    /**
     * @brief Construct a new Treatment object
     *
     * On construction of a new Treatment object the _s_timestamp is set to its
     * initial value, a random cookie_secret is calculated and the pointers to
     * both PacketContainers are being stored in member variables.
     * @param pkt_to_inside PacketContainer containing the packets with the
     * destination being the secured network
     * @param pkt_to_outside PacketContainer containing the packets with the
     * destination being the internet
     */
    inline Treatment(MbufContainerTransmitting* pkt_send_to_outside,
                     MbufContainerTransmitting* pkt_send_to_inside,
                     u_int8_t syn_thresh)
        : _cookie_secret(Rand::get_random_64bit_value())
        , _packet_to_inside(pkt_send_to_inside)
        , _packet_to_outside(pkt_send_to_outside)
        , _syn_thresh(syn_thresh)
        , _skip_syn(0) {

        // disables logging
        /*  auto corehandle = boost::log::core::get();
         corehandle->set_logging_enabled(false); */

        // Densemap requires you to set an empty key, which is never used by
        // legit connections Dense_hash_map requires you call set_empty_key()
        _densemap.set_empty_key(Data(0, 0, 0, 0));

        // immediately after constructing the hash_map, and before calling any
        // other dense_hash_map method
        _densemap.set_deleted_key(Data(0, 0, 1, 1));
        // BOOST_LOG_TRIVIAL(info) << "|---Init of Treatment done. Created
        // cookie_secret, " "_s_timestamp and inserted empty and deleted
        // key---|";
    }

    /**
     * @brief Construct a new Treatment object for testing purposes, create a
     * random cookie_secret and init the _s_timestamp
     *
     */
    inline Treatment() {
        _cookie_secret = 0;
        _s_timestamp = 0;
    }

    /**
     * @brief This method checks if the packet to inside is suitable for
     * treatment and treats it accordingly
     *
     * The function treat_packets_to_inside() works on two packetcontainers and
     * iterates over all elements inside of them. If the packet is not yet
     * deleted and if it is a TCPIPv4 packet, then treatment begins. Depending
     * on the flag combinations in the TCPHeader further steps are being taken
     * in order to fulfill the requirements, which are defined in the activity
     * diagram "treat_packets()" as seen in the review document. treat_packets()
     * involves adjustments of sequence and acknowledgement numbers, calculating
     * and checking syn-cookies. Depending on internal rules packets can be
     * forwarded, adjusted, stored and dropped on their way through the system.
     */
    inline bool treat_packets_to_inside(PacketInfoIpv4Tcp* _pkt_info) {

        // BOOST_LOG_TRIVIAL(info)<< "Size of densemap before to inside: " <<
        // _densemap.size();

        /************************************************************************
         ****Treatment of packets with direction towards the internal
         *network****
         ************************************************************************/
        // get packetInfo at current position
        // check if packet has been deleted
        // check wether packet is IPv4TCP
        // get the flags of packet i as they will be needed a few times
        u_int8_t _flags = _pkt_info->get_flags();
        // SYN-ACK is set, simply forward the packet to the internal
        // network and create an entry in the _densemap
        if ((_flags & _flag_enum::SYNFIN) == _flag_enum::SYNFIN) {
            rte_pktmbuf_free(_pkt_info->get_mbuf());
        } else if ((_flags & _flag_enum::SYNACK) ==
                   _flag_enum::SYNACK) { // check wether the syn and ack
                                         // flag is set
            // BOOST_LOG_TRIVIAL(info) << "|---Received SYN-ACK from
            // outside---|";
            // just simply forward the packet to the internal network
            // also create entry in the _densemap, with _offset
            // difference =
            // 0
            Data insert(_pkt_info->get_src_ip(), _pkt_info->get_dst_ip(),
                        _pkt_info->get_src_port(), _pkt_info->get_dst_port());
            Info info(0, false, false, false, false, nullptr);
            _densemap.insert(std::make_pair(insert, info));
            // BOOST_LOG_TRIVIAL(info)  << "|---Created new entry for
            // connection: _extip  " << _pkt_info->get_src_ip() << " _extport: "
            // << _pkt_info->get_src_port()<< " _intip: " <<
            // _pkt_info->get_dst_ip() << " _intport: " <<
            // _pkt_info->get_dst_port() << "---|"; place packet in the sending
            // container to inside
            _packet_to_inside->add_mbuf(_pkt_info->get_mbuf());

        }
        // SYN is set, generate cookie hash
        else if ((_flags & _flag_enum::SYN) == _flag_enum::SYN) {

            if (_skip_syn < _syn_thresh) {
                ++_skip_syn;
            } else { // SYN to INSIDE
                // BOOST_LOG_TRIVIAL(info) << "|---Received SYN to inside---|";
                // BOOST_LOG_TRIVIAL(info) << "|---Mbuf is: " <<
                // _pkt_info->get_mbuf() << "---|";
                /* use this part to calculate the syn-cookie, create a new
                 * packet and send it back to the same side it came from */
                u_int32_t _cookie = calc_cookie_hash(
                    _s_timestamp, _pkt_info->get_src_ip(),
                    _pkt_info->get_dst_ip(), _pkt_info->get_src_port(),
                    _pkt_info->get_dst_port());
                // BOOST_LOG_TRIVIAL(info) << "|---Calculated cookie is: " <<
                // _cookie << "---|";

                _cookie = _cookie & 0xFFFFFF00; // got upper bits
                u_int32_t _seqnum =
                    _cookie |
                    _s_timestamp; // got cookie + 8 bit _s_timestamp
                                  // BOOST_LOG_TRIVIAL(info)<<
                                  // "|---Calculated cookie with timestamp
                                  // is: " << _seqnum << "---|";

                // create the reply for the external connection/host
                rte_mbuf* reply_mbuf = _packet_to_outside->get_empty_mbuf();
                if (reply_mbuf == nullptr) {
                    return false;
                }
                rte_pktmbuf_append(reply_mbuf, sizeof(struct rte_ether_hdr) +
                                                   sizeof(struct rte_ipv4_hdr) +
                                                   sizeof(struct rte_tcp_hdr));
                PacketInfoIpv4Tcp* reply = new PacketInfoIpv4Tcp(reply_mbuf);
                // BOOST_LOG_TRIVIAL(info) << "|---The reply is of type: " <<
                // reply->get_type() << "---|";

                reply->fill_payloadless_tcp_packet(
                    _pkt_info->get_dst_mac(), _pkt_info->get_src_mac(),
                    _pkt_info->get_dst_ip(), _pkt_info->get_src_ip(),
                    _pkt_info->get_dst_port(), _pkt_info->get_src_port(),
                    _seqnum, _pkt_info->get_seq_num() + 1, _flag_enum::SYNACK,
                    _pkt_info->get_window_size());
                reply->prepare_offloading_checksums();
                // BOOST_LOG_TRIVIAL(info) << "|---Created SYN-ACK with Cookie:
                // " << reply->get_seq_num()<< "---|"; BOOST_LOG_TRIVIAL(info)
                // << " |---The new SYN-ACK goes with: extip: " <<
                // reply->get_dst_ip()<< " extport: " << reply->get_dst_port()<<
                // " intip: " << reply->get_src_ip() << " intport: " <<
                // reply->get_src_port() << "---|"; delete/drop received
                rte_pktmbuf_free(_pkt_info->get_mbuf());
                // BOOST_LOG_TRIVIAL(info) << "|---Dropped incoming packet---|";
                // now packet is getting send out in the next burst cycle

                delete reply;
                _skip_syn = 0;
            }

        }
        // Case: RST received from outside
        else if ((_flags & _flag_enum::RST) == _flag_enum::RST) {
            // BOOST_LOG_TRIVIAL(info) << "|---Received RST from outside,
            // ""deleting entry in densemap---|"; erase entry in _densemap with
            // key = hash(AliceIP, AlicePort, BobIP, BobPort) Send packet to
            // inside with destIP = BobIP, destPort = BobPort, srcIP = AliceIP,
            // srcPort = AlicePort erased entry in _densemap
            _densemap.erase(
                Data(_pkt_info->get_src_ip(), _pkt_info->get_dst_ip(),
                     _pkt_info->get_src_port(), _pkt_info->get_dst_port()));

            _packet_to_inside->add_mbuf(_pkt_info->get_mbuf());

        }
        // Case: Received FIN, no FIN-ACK from outside
        else if (((_flags & _flag_enum::FIN) == _flag_enum::FIN)) {
            // BOOST_LOG_TRIVIAL(info) << "|---Received a FIN from outside---|";
            // create Data fin for the packet
            // find the entry in our map
            auto iter = _densemap.find(
                Data(_pkt_info->get_src_ip(), _pkt_info->get_dst_ip(),
                     _pkt_info->get_src_port(), _pkt_info->get_dst_port()));
            // Catch case, that we dont interact on empty map
            if (iter == _densemap.end()) {
                // drop packet
                rte_pktmbuf_free(_pkt_info->get_mbuf());
                // BOOST_LOG_TRIVIAL(info)<< "|---Dropped Packet comming from
                // outside---|";
            }

            // Check if fin has already been seen, and we do not have an
            // finack
            else if (iter->second._finseen_to_outside == true) {
                // BOOST_LOG_TRIVIAL(info) << "|---FIN-ACK to FIN received from
                // outside---|"; change the packets ack number tweak by adding
                // into one line
                iter->second._finseen_to_inside = true;
                iter->second._ack_to_outside_expected = true;
                _pkt_info->set_ack_num(_pkt_info->get_ack_num() -
                                       iter->second._offset);
                _pkt_info->prepare_offloading_checksums();

                // delete the entry
                /*  _densemap.erase(Data(
                     _pkt_info->get_src_ip(), _pkt_info->get_dst_ip(),
                     _pkt_info->get_src_port(),
                 _pkt_info->get_dst_port()));
                 //BOOST_LOG_TRIVIAL(info) << "Deleted entry in densemap";
               */

                // send packet to inside // aka. do nothing
                _packet_to_inside->add_mbuf(_pkt_info->get_mbuf());

            } else if (iter->second._finseen_to_inside == false) {
                // change status _finseen to true
                iter->second._finseen_to_inside = true;
                // change the packets ack number
                _pkt_info->set_ack_num(_pkt_info->get_ack_num() -
                                       iter->second._offset);
                // refresh the checksums
                _pkt_info->prepare_offloading_checksums();
                // BOOST_LOG_TRIVIAL(info) << "|---Changed Finseen to true for
                // connection: ""extip: " << _pkt_info->get_src_ip()
                //<< " extport: " << _pkt_info->get_src_port()
                //<< " intip: " << _pkt_info->get_dst_ip()
                //<< " intport: " << _pkt_info->get_dst_port() << "---|";

                _packet_to_inside->add_mbuf(_pkt_info->get_mbuf());
            } else {
                // BOOST_LOG_TRIVIAL(info) << "|---Duplicate FIN from
                // Outside---|";
                _pkt_info->set_ack_num(_pkt_info->get_ack_num() -
                                       iter->second._offset);
                // refresh the checksums
                _pkt_info->prepare_offloading_checksums();
                _packet_to_inside->add_mbuf(_pkt_info->get_mbuf());
            }

        }
        // CASE: ACK RECEIVED from Alice
        else if ((_flags & _flag_enum::ACK) == _flag_enum::ACK &&
                 ((_flags & _flag_enum::FINACK) != _flag_enum::FINACK)) {
            // BOOST_LOG_TRIVIAL(info) << "|---Received ACK from outside---|";
            Data ack(_pkt_info->get_src_ip(), _pkt_info->get_dst_ip(),
                     _pkt_info->get_src_port(), _pkt_info->get_dst_port());
            auto id = _densemap.find(ack);
            if (id == _densemap.end()) { // no entry is here yet
                                         // BOOST_LOG_TRIVIAL(info)
                // << "|---Received ACK to SYN-ACK from outside---|";
                // BOOST_LOG_TRIVIAL(info) << "|---Checking the SYN-COOKIE---|";
                u_int32_t _cookie_val =
                    _pkt_info->get_ack_num(); // get acknowledgementnumber
                                              // from packet
                bool _legit =
                    check_syn_cookie(_cookie_val - 1,
                                     ack); // check for the correkt cookie
                if (_legit == true) { // the connection is to be established
                    // Take the packet and save it, till the connection
                    // to the internal host is established Use the
                    // densemap for this purpose by setting the pointer
                    // in info to taken_packet
                    rte_mbuf* reply_mbuf = _packet_to_inside->get_empty_mbuf();
                    if (reply_mbuf == nullptr) {
                        return false;
                    }
                    rte_pktmbuf_append(reply_mbuf,
                                       sizeof(struct rte_ether_hdr) +
                                           sizeof(struct rte_ipv4_hdr) +
                                           sizeof(struct rte_tcp_hdr));
                    PacketInfoIpv4Tcp* reply =
                        new PacketInfoIpv4Tcp(reply_mbuf);

                    if (reply == nullptr) {
                        // BOOST_LOG_TRIVIAL(fatal) << "created a nullptr";
                    }
                    _densemap.insert(std::make_pair(
                        ack, Info(0, false, false, false, false, _pkt_info)));
                    // Still need to send that syn to inside

                    // TO-DO check if _pkt_info is legit there
                    reply->fill_payloadless_tcp_packet(
                        _pkt_info->get_src_mac(), _pkt_info->get_dst_mac(),
                        _pkt_info->get_src_ip(), _pkt_info->get_dst_ip(),
                        _pkt_info->get_src_port(), _pkt_info->get_dst_port(),
                        _pkt_info->get_seq_num() - 1, 123, _flag_enum::SYN,
                        _pkt_info->get_window_size());
                    reply->prepare_offloading_checksums();
                    // BOOST_LOG_TRIVIAL(info)
                    // << "|---Cookie " << _pkt_info->get_ack_num() - 1
                    // << " correct, sending SYN to inside---|";
                    // BOOST_LOG_TRIVIAL(info)
                    //<< "|---SYN to inside goes from " << reply->get_src_ip()
                    // << " to: " << reply->get_dst_ip() << " ---|";

                    // now packet is getting send out in the next burst
                    // cycle
                    _packet_to_outside->add_mbuf(reply_mbuf);
                    delete reply;
                    return true;

                } else if (_legit == false) { // the connection is closed
                                              // with an rst, drop packet
                                              // BOOST_LOG_TRIVIAL(info)
                    // << "|---Cookie incorrect, dropped packet---|";

                    // Send RST if (diff>1 XOR hash!=cookie_value)
                    rte_mbuf* rst_mbuf = _packet_to_inside->get_empty_mbuf();
                    if (rst_mbuf == nullptr) {
                        return false;
                    }
                    PacketInfo* rst =
                        PacketInfoCreator::create_pkt_info(rst_mbuf, IPv4TCP);
                    PacketInfoIpv4Tcp* rst4 =
                        static_cast<PacketInfoIpv4Tcp*>(rst);

                    rst4->fill_payloadless_tcp_packet(
                        _pkt_info->get_dst_mac(), _pkt_info->get_src_mac(),
                        ack._extip, ack._intip, ack._extport, ack._intport,
                        _pkt_info->get_ack_num(), 0, _flag_enum::RST,
                        _pkt_info
                            ->get_window_size()); // fill_payloadless_tcp_packet
                                                  // already exists, we
                                                  // just need it from
                                                  // Tobias
                    rst4->prepare_offloading_checksums();
                    _packet_to_outside->add_mbuf(rst4->get_mbuf());

                    rte_pktmbuf_free(_pkt_info->get_mbuf());
                }

            }
            // No entry in _densemap -> It is an ACK as a reply to
            // SYN-ACK entry can be found and its not a reply to an fin
            // ack
            else if (unlikely(id->second._finseen_to_inside == true &&
                              id->second._finseen_to_outside == true &&
                              id->second._ack_to_inside_expected == true)) {
                // BOOST_LOG_TRIVIAL(info)
                //  << "|---ACK to FIN-ACK, Connection Closed---|";
                // simply manage _offset and thats it
                _pkt_info->set_ack_num(_pkt_info->get_ack_num() -
                                       id->second._offset);
                _pkt_info->prepare_offloading_checksums();

                // delete the entry
                _densemap.erase(id);
                // BOOST_LOG_TRIVIAL(info) << "|---Deleted entry in
                // densemap---|";

                _packet_to_inside->add_mbuf(_pkt_info->get_mbuf());

            }
            // If the connection from inside is not fully established
            // yet, i need to add it to the queue first
            else if (unlikely(id->second._pkt_inf_list.empty() == false)) {
                // now add it to the queue
                // BOOST_LOG_TRIVIAL(info) << "|---Put early ack into
                // queue---|";
                id->second._pkt_inf_list.push_back(_pkt_info);
                return true;

            } else {
                // BOOST_LOG_TRIVIAL(info) << "|---Regular ACK, simply
                // send---|";
                // simply manage _offset and thats it
                _pkt_info->set_ack_num(_pkt_info->get_ack_num() -
                                       id->second._offset);
                _pkt_info->prepare_offloading_checksums();

                _packet_to_inside->add_mbuf(_pkt_info->get_mbuf());
            }
        }
        // BOOST_LOG_TRIVIAL(info) << "Size of densemap after to inside: " <<
        // _densemap.size();
        return false;
    }
    /**
     * @brief This method checks if the packet to outside is suitable for
     * treatment and treats it accordingly
     *
     * The function treat_packets_to_outside() works on two packetcontainers and
     * iterates over all elements inside them. If the packet is not yet deleted
     * and if it is a TCPIPv4 packet, then treatment begins. Depending on the
     * flag combinations in the TCPHeader further steps are being taken in order
     * to fulfill the requirements, which are defined in the activity diagram
     * "treat_packets()" as seen in the review document. treat_packets()
     * involves adjustments of sequence and acknowledgement numbers, calculating
     * and checking syn-cookies. Depending on internal rules packets can be
     * forwarded, adjusted, stored and dropped on their way through the system.
     */
    inline bool treat_packets_to_outside(PacketInfoIpv4Tcp* _pkt_info) {

        // BOOST_LOG_TRIVIAL(info) << "Size of densemap before to outside: " <<
        // _densemap.size();
        /************************************************************************
        ****Treatment of packets with direction towards the external network****
        ************************************************************************/
        u_int8_t _flags =
            _pkt_info->get_flags(); // get the flags of packet as
                                    // they will be needed a few times
        // BOOST_LOG_TRIVIAL(info) << "THE TYPE IS" << std::to_string(_flags);
        if ((_flags & _flag_enum::SYNFIN) == _flag_enum::SYNFIN) {
            rte_pktmbuf_free(_pkt_info->get_mbuf());
        } else if ((_flags & _flag_enum::SYNACK) ==
                   _flag_enum::SYNACK) { // check wether the syn&ack flag is
                                         // set
                                         // BOOST_LOG_TRIVIAL(info) << "|---Got
                                         // a SYN-ACK comming from inside---|";
            // Create entry in the Densemap
            // Reply with the ack stored in _densemap for this specific
            // connection...

            // Get Data from connection in order to create entry in
            // _densemap and check _densemap
            Data dfirst(_pkt_info->get_dst_ip(), _pkt_info->get_src_ip(),
                        _pkt_info->get_dst_port(), _pkt_info->get_src_port());
            // find according value in _densemap
            auto id = _densemap.find(dfirst);
            // unsure about that, was to catch the case, that no ack has
            // ever been held
            if (id == _densemap.end()) {
                // drop packet and delete connection info
                rte_pktmbuf_free(_pkt_info->get_mbuf());
            } else if (id->second._pkt_inf_list.empty() == false) {
                // cast into an ipv4 packet

                bool first_packet = true;
                for (auto elem : id->second._pkt_inf_list) {

                    if (first_packet) {

                        // calculate _offset by substracting
                        // internal_ack_num from external_ack_num
                        int offset =
                            elem->get_ack_num() - _pkt_info->get_seq_num() - 1;

                        // BOOST_LOG_TRIVIAL(info)  << "|---The offset is:" <<
                        // offset << ", the external acknum is: " <<
                        // elem->get_ack_num() << " the internal seqnum is: " <<
                        // _pkt_info->get_seq_num() << " --- |";

                        id->second._offset = offset;
                        // adjust the acknum of the ack as a response to
                        // syn ack
                        // outside->set_ack_num(outside->get_ack_num() -
                        // offset);
                        elem->set_ack_num(_pkt_info->get_seq_num() + 1);
                        // refresh checksum
                        elem->prepare_offloading_checksums();
                        // send received ack with adjusted acknum, this
                        // may already contain data
                        _packet_to_inside->add_mbuf(elem->get_mbuf());
                        // erase entry in _pkt_inf

                        first_packet = false;
                    } else {

                        elem->set_ack_num(elem->get_ack_num() -
                                          id->second._offset);

                        elem->prepare_offloading_checksums();
                        _packet_to_inside->add_mbuf(elem->get_mbuf());
                    }
                    delete elem;
                }
                id->second._pkt_inf_list.clear();
            } else {
                // BOOST_LOG_TRIVIAL(fatal) << "No element in the list";
            }
        }
        // check wether the syn flag is set
        else if ((_flags & _flag_enum::SYN) == _flag_enum::SYN) {
            // Simply forward the packet
            // BOOST_LOG_TRIVIAL(info) << "|---Forwarding the " "SYN from
            // internal to external ---|";
            _packet_to_outside->add_mbuf(_pkt_info->get_mbuf());
        }
        // RST received from inside_pkt_
        else if ((_flags & _flag_enum::RST) == _flag_enum::RST) {
            // BOOST_LOG_TRIVIAL(info) << "|---Received an RST from inside---|";
            Data erase(_pkt_info->get_dst_ip(), _pkt_info->get_src_ip(),
                       _pkt_info->get_dst_port(), _pkt_info->get_src_port());
            auto id = _densemap.find(erase);
            if (id == _densemap.end()) {
                // drop packet
                rte_pktmbuf_free(_pkt_info->get_mbuf());

            } else {
                _pkt_info->set_seq_num(_pkt_info->get_seq_num() +
                                       id->second._offset);
                _pkt_info->prepare_offloading_checksums();

                _packet_to_outside->add_mbuf(_pkt_info->get_mbuf());

                _densemap.erase(erase);
            }
        }
        // FIN received from inside_pkt
        else if (((_flags & _flag_enum::FIN) == _flag_enum::FIN)) {
            // BOOST_LOG_TRIVIAL(info) << "|---Recevied a FIN from inside---|";
            // create Data fin for the packet

            // find the entry in our map
            auto iter = _densemap.find(
                Data(_pkt_info->get_dst_ip(), _pkt_info->get_src_ip(),
                     _pkt_info->get_dst_port(), _pkt_info->get_src_port()));
            if (iter == _densemap.end()) {
                // drop packet
                rte_pktmbuf_free(_pkt_info->get_mbuf());
            }
            else if ((iter->second._finseen_to_inside == true) /* &&
                         ((_flags & _flags::FINACK) == _flags::FINACK)*/) {
                // Check if fin has already been seen and we are not
                // looking for the reply
                // BOOST_LOG_TRIVIAL(info) << "|---FIN-ACK for FIN---|";
                // change the packets seq number
                iter->second._finseen_to_outside = true;
                iter->second._ack_to_inside_expected = true;
                _pkt_info->set_seq_num(_pkt_info->get_seq_num() +
                                       iter->second._offset);
                _pkt_info->prepare_offloading_checksums();
                // delete the entry
                /*  _densemap.erase(Data(
                     _pkt_info->get_dst_ip(), _pkt_info->get_src_ip(),
                     _pkt_info->get_dst_port(),
                   _pkt_info->get_src_port()));
                 */
                // send packet to inside // aka. do nothing

                _packet_to_outside->add_mbuf(_pkt_info->get_mbuf());
            }

            // Case: its the first FIN with piggybagged ack
            else if (iter->second._finseen_to_outside == false) {
                // BOOST_LOG_TRIVIAL(info)<< "|---Recevied the first FIN from
                // inside---|"; change status _finseen to true
                iter->second._finseen_to_outside = true;
                // get _offset
                // change the packets seq number
                _pkt_info->set_seq_num(_pkt_info->get_seq_num() +
                                       iter->second._offset);
                _pkt_info->prepare_offloading_checksums();
                // BOOST_LOG_TRIVIAL(info)  << "|---Changed Finseen to true for
                // connection: "   "extip: "
                /*  << _pkt_info->get_dst_ip()
                 << " extport: " << _pkt_info->get_dst_port()
                 << " intip: " << _pkt_info->get_src_ip()
                 << " intport: " << _pkt_info->get_src_port() << "---|"; */

                _packet_to_outside->add_mbuf(_pkt_info->get_mbuf());
            } else {
                // BOOST_LOG_TRIVIAL(info)  << "|--- Saw duplicate FIN from
                // outside----|";

                _pkt_info->set_seq_num(_pkt_info->get_seq_num() +
                                       iter->second._offset);
                _pkt_info->prepare_offloading_checksums();
                _packet_to_outside->add_mbuf(_pkt_info->get_mbuf());
            }
        }

        // CASE: ACK RECEIVED from Bob
        else if (((_flags & _flag_enum::ACK) == _flag_enum::ACK) &&
                 ((_flags & _flag_enum::FINACK) != _flag_enum::FINACK)) {

            // BOOST_LOG_TRIVIAL(info) << "|---Received an ACK from inside---|";
            Data ack(_pkt_info->get_dst_ip(), _pkt_info->get_src_ip(),
                     _pkt_info->get_dst_port(), _pkt_info->get_src_port());
            auto id = _densemap.find(ack);
            // no entry in _densemap, create one and adjust value
            // accordingly
            if (id == _densemap.end()) {
                // BOOST_LOG_TRIVIAL(info) << "|---Created new entry in the
                // densemap---|";
                Info info(0, false, false, false, false, nullptr);
                _densemap.insert(std::make_pair(ack, info));

                // send out packet, aka do nothing
                _packet_to_outside->add_mbuf(_pkt_info->get_mbuf());

            } else if (unlikely(id->second._finseen_to_inside == true &&
                                id->second._finseen_to_outside == true &&
                                id->second._ack_to_outside_expected == true)) {
                // BOOST_LOG_TRIVIAL(info) << "|---ACK to FIN-ACK-Packet---|";

                _pkt_info->set_seq_num(_pkt_info->get_seq_num() +
                                       id->second._offset);
                _pkt_info->prepare_offloading_checksums();

                _densemap.erase(id);
                // BOOST_LOG_TRIVIAL(info) << "|---Deleted entry in
                // densemap--|";
                _packet_to_outside->add_mbuf(_pkt_info->get_mbuf());

            } else {
                // BOOST_LOG_TRIVIAL(info) << "|---Just a regular
                // ACK-Packet---|";
                // it is just a regular packet, simply adjust with
                // _offset and you are done.
                _pkt_info->set_seq_num(_pkt_info->get_seq_num() +
                                       id->second._offset);
                _pkt_info->prepare_offloading_checksums();

                _packet_to_outside->add_mbuf(_pkt_info->get_mbuf());
            }
        }
        // BOOST_LOG_TRIVIAL(info) << "Size of densemap after to outside: " <<
        // _densemap.size();
        return false;
    }

    /**
     * @brief used in the main_lcore timer, in order to increment the
     * _s_timestamp every 64 seconds This function does nothing besides
     * incrementing the _s_timestamp by one every time it is called from our
     * thread
     * @param timestamp is global, but changed by this function
     */
    inline static void s_increment_timestamp() {
        // increment _s_timestamp by one
        ++_s_timestamp;
    }

  private:
    /**
     * @brief Calculates the Cookiehash from the global _s_timestamp, a global
     * cookie_secret and unique connection identifiers
     *
     * @param _s_timestamp is global
     * @param _extip
     * @param _intip
     * @param _extport
     * @param _intport
     * @return u_int32_t
     */
    inline u_int32_t calc_cookie_hash(u_int8_t _s_timestamp, u_int32_t _extip,
                                      u_int32_t _intip, u_int16_t _extport,
                                      u_int16_t _intport) {

        // use XXH3_64bits as shrinking it is still faster than using XXH32
        return XXH3_64bits_withSeed(
            &_s_timestamp, sizeof(_s_timestamp),
            XXH3_64bits_withSeed(
                &_extip, sizeof(_extip),
                XXH3_64bits_withSeed(
                    &_intip, sizeof(_intip),
                    XXH3_64bits_withSeed(
                        &_extport, sizeof(_extport),
                        XXH3_64bits_withSeed(&_intport, sizeof(_intport),
                                             _cookie_secret)))));
    }
    /**
     * @brief Check if the syn_cookie was received in the correct timespan and
     * if the reveiced cookie is correct/like the expected cookie
     *
     * @param cookie_value This is the sequencenumber sent away in the syn-ack
     * @param d d is the data we obtained from our packet, needed to calculate
     * the expected hash
     *
     */
    inline bool check_syn_cookie(u_int32_t cookie_value, const Data& d) {
        // Extract the last 8 bits of the cookie (= timestamp)
        u_int8_t cookie_timestamp = cookie_value & 0x000000FF;

        u_int8_t diff = _s_timestamp - cookie_timestamp;

        if (diff <= 1) {
            // Calculate hash
            u_int32_t hash;

            // Case: same time interval
            if (diff == 0) {
                // calculate expected cookie_hash
                hash = calc_cookie_hash(_s_timestamp, d._extip, d._intip,
                                        d._extport, d._intport);
                hash = hash & 0xFFFFFF00;
                // stuff cookie_hash with 8 bit _s_timestamp
                hash |= (u_int8_t)_s_timestamp;
            }

            if (diff == 1) {
                // calculate expected cookie_hash for older timeinterval
                hash = calc_cookie_hash((u_int8_t)(_s_timestamp - 1), d._extip,
                                        d._intip, d._extport, d._intport);
                hash = hash & 0xFFFFFF00;
                // stuff cookie with 8 bit _s_timestamp
                hash |= (u_int8_t)(_s_timestamp - 1);
            }

            // test wether the cookie is as expected; if so, return true
            if (hash == cookie_value) {
                return true;
            }
        }
        // return false, so that treat_packets is able to continue
        return false;
    }

    inline static u_int8_t _s_timestamp =
        0; ///< timestamp used to check the legitimacy of SYN-cookies

    u_int64_t _cookie_secret; ///< cookie_secret used to enhance the efficency
                              ///< of SYN-cookies
    u_int8_t _skip_syn;

    u_int8_t _syn_thresh;

    MbufContainerTransmitting*
        _packet_to_inside; ///< MbufContainer containing packets with
                           ///< destination being the internal network

    MbufContainerTransmitting*
        _packet_to_outside; ///< MbufContainer containing packets with
                            ///< destination being the extern network

    google::dense_hash_map<Data, Info, TreatmentHash>
        _densemap; ///< Map to store information about connections including
                   ///< _offset and if a fin has been seen

    friend class Treatment_friend; ///< used for unit tests
};
