/**
 * @file ForwardingThread.hpp
 * @author Jakob
 * @brief 
 * @version 0.1
 * @date 2021-07-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include "PacketDissection/PacketContainer.hpp"
#include "PacketDissection/PacketContainerLean.hpp"

#include "Threads/Thread.hpp"

class ForwardingThread : public Thread {
  public:
    /**
     * @brief Construct a new Thread object
     *
     * @param pkt_container_to_inside PacketContainer of packets that come from
     * the outside port and are to be forwarded to the inside port
     * @param pkt_container_to_outside PacketContainer of packets that come from
     * the inside port and are to be forwarded to the outside port
     */
    ForwardingThread(PacketContainer* pkt_container_to_inside,
                     PacketContainer* pkt_container_to_outside)
        : Thread(), _pkt_container_to_inside(pkt_container_to_inside),
          _pkt_container_to_outside(pkt_container_to_outside) {}

    ForwardingThread(PacketContainerLean* pkt_container_to_inside,
                     PacketContainerLean* pkt_container_to_outside)
        : Thread(), _pkt_container_to_inside_lean(pkt_container_to_inside),
          _pkt_container_to_outside_lean(pkt_container_to_outside) {}

  protected:
    /**
     * @brief in AttackThread: to dave
     */
    PacketContainer* _pkt_container_to_inside;
    PacketContainerLean* _pkt_container_to_inside_lean;

    /**
     * @brief in AttackThread: to alice
     */
    PacketContainer* _pkt_container_to_outside;
    PacketContainerLean* _pkt_container_to_outside_lean;
};
