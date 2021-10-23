#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#include "Definitions.hpp"

#include "Threads/DefenseThread.hpp"

// ===== PUBLIC ===== //

DefenseThread::DefenseThread(
    MbufContainerReceiving* mbuf_container_from_inside,
    MbufContainerReceiving* mbuf_container_from_outside,
    MbufContainerTransmitting* mbuf_container_to_inside,
    MbufContainerTransmitting* mbuf_container_to_outside)
    : Thread()
    , _mbuf_container_from_inside(mbuf_container_from_inside)
    , _mbuf_container_from_outside(mbuf_container_from_outside)
    , _mbuf_container_to_inside(mbuf_container_to_inside)
    , _mbuf_container_to_outside(mbuf_container_to_outside)
    , _do_treat(false) {

    _treatment =
        new Treatment(_mbuf_container_to_outside, _mbuf_container_to_inside, 0);
}

int DefenseThread::s_run(void* thread_vptr) {
    DefenseThread* thread = static_cast<DefenseThread*>(thread_vptr);
    thread->run();
    return 0;
}

// ===== PRIVATE ===== //

void DefenseThread::run() {
    /*
    LOG_INFO << "\nRunning on lcore " << rte_lcore_id() << ". [Ctrl+C to quit]"
             << LOG_END;
             */

    // Run until the application is quit or killed.
    while (likely(_quit == false)) {

        // std::cout << _do_treat << std::endl;

        // continue if no packets are received
        int mbufs_from_inside = _mbuf_container_from_inside->poll_mbufs();
        /*  if (mbufs_from_inside != 0) {
             BOOST_LOG_TRIVIAL(info)
                 << "Number of packets received from inside: "
                 << mbufs_from_inside;
         } */
        bool keep = true;
        for (int i = 0; i < mbufs_from_inside; ++i) {
            rte_mbuf* mbuf = _mbuf_container_from_inside->get_mbuf_at_index(i);

            if (PacketInfoCreator::is_ipv4_tcp(mbuf) == true &&
                _do_treat == true) {
                PacketInfoIpv4Tcp* pkt_info = new PacketInfoIpv4Tcp(mbuf);
                keep = _treatment->treat_packets_to_outside(pkt_info);
                if (keep == false) {
                    // delete PacketInfo
                    delete pkt_info;
                }
            } else {
                // fwd
                // std::cout << _do_treat;
                _mbuf_container_to_outside->add_mbuf(mbuf);
            }
        }

        _mbuf_container_to_inside->send_mbufs();
        _mbuf_container_to_outside->send_mbufs();

        // _mbuf_container_to_inside->send_mbufs();
        // _mbuf_container_to_outside->send_mbufs();

        // for mbuf in pktsFromOutside
        //    pktInfoCreator::determinePacketType without creating a pktinfo
        //    create packetInfo with obtained type
        //    if tcp: treat
        //    else: fwd
        //    destroy pktInfo
        // Pakete von innen, auch die selbsterzeugten bevorzugen

        // continue if no packets are received

        int mbufs_from_outside = _mbuf_container_from_outside->poll_mbufs();
        /*  if (mbufs_from_outside != 0) {
             BOOST_LOG_TRIVIAL(info)
                 << "Number of packets received from outside: "
                 << mbufs_from_outside;
         } */
        for (int i = 0; i < mbufs_from_outside; ++i) {
            rte_mbuf* mbuf = _mbuf_container_from_outside->get_mbuf_at_index(i);

            if (PacketInfoCreator::is_ipv4_tcp(mbuf) == true &&
                _do_treat == true) {
                PacketInfoIpv4Tcp* pkt_info = new PacketInfoIpv4Tcp(mbuf);
                keep = _treatment->treat_packets_to_inside(
                    static_cast<PacketInfoIpv4Tcp*>(pkt_info));
                if (keep == false) {
                    // delete PacketInfo
                    delete pkt_info;
                }
            } else {
                // std::cout << _do_treat;
                _mbuf_container_to_inside->add_mbuf(mbuf);
            }
        }
        /*   if (_mbuf_container_to_inside->get_number_of_mbufs() > 0) {
              BOOST_LOG_TRIVIAL(info)
                  << "Number of packets sent to inside: "
                  << _mbuf_container_to_inside->get_number_of_mbufs();
          }
          if (_mbuf_container_to_outside->get_number_of_mbufs() > 0) {
              BOOST_LOG_TRIVIAL(info)
                  << "Number of packets sent to outside: "
                  << _mbuf_container_to_outside->get_number_of_mbufs();
          } */

        _mbuf_container_to_inside->send_mbufs();
        _mbuf_container_to_outside->send_mbufs();
    }

    _running = false;
}
