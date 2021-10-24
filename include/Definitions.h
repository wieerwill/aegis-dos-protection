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
#define NUM_NON_WORKER_THREADS 2

// used in PacketContainer.
// predefined size of array of mbuf-arrays
#define NUM_MBUF_ARRS 5000

#define TCP_RX_WINDOW 16384
