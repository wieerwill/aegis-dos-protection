/**
 * @file Definitions.h
 * @author Jakob
 * @brief 
 * @version 0.1
 * @date 2021-07-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

// only needed for mempool creation. was replaced.
// NUM_MBUF_POOL_ELEMENTS is used instead
//#define NUM_MBUFS 8191

// Argument "n" in rte_pktmbuf_pool_create
#define NUM_MBUF_POOL_ELEMENTS 32767

#define BURST_SIZE 32

#define RSS_HASH_KEY_LENGTH 40

// used in initializer
#define NUM_NON_WORKER_THREADS 1

// used in PacketContainer.
// predefined size of array of mbuf-arrays
#define NUM_MBUF_ARRS 30000

#define TCP_RX_WINDOW 16384

#define LOG_INFO BOOST_LOG_TRIVIAL(info) << "\e[32m"
#define LOG_WARNING BOOST_LOG_TRIVIAL(warning) << "\e[33m "
#define LOG_ERROR BOOST_LOG_TRIVIAL(error) << "\e[31m "
#define LOG_FATAL BOOST_LOG_TRIVIAL(fatal) << "\e[31m "
#define LOG_END "\e[0m"

// ATTACKE
//#define SINGLE_ITERATION
//#define LOG_PKTS_SENT
//#define SEND_ONCE
//#define SEND_PKTS_EVERY_NTH_ITERATION 1000