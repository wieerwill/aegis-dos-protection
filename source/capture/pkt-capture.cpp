/**
 * @file pkt-capture.cpp
 *
 * @brief Capture packets with dpdk
 *
 **/

#include <inttypes.h>
#include <rte_cycles.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <stdint.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

/**
 * @brief set port configuration
 *
 */

// "Portconfig", given by DPDK.
// this struct is a data type with which you have to configure the NIC
// first you create the configuration file which later can be applicated to the
// NIC
static const struct rte_eth_conf port_conf_default = {
    // TODO: what does this dot mean?
    // the structure of this config-struct is given by the DPDK itself
    .rxmode =
        {
            .max_rx_pkt_len = RTE_ETHER_MAX_LEN,
        },
};

/**
 * @brief init NIC
 *
 * get the NIC by port identifier and bind it to dpdk with given mempool.
 * Setting up TX/RX queues per ethernet port. Then starting the device and
 * printing MAC address
 *
 * @param port set port identifier to use
 * @param mbuf_pool set mempool of dpdk to use
 * @return 0 or error code
 */
static inline int port_init(uint16_t port, struct rte_mempool* mbuf_pool) {
    // the "port" is a port_id which is fetched in the main-function by the
    // Makro RTE_ETH_FOREACH_DEV The mbuf_pool is needed for queue
    // initialisation

    // config
    struct rte_eth_conf port_conf = port_conf_default;

    // number of rx and tx queues (i guess)
    const uint16_t rx_rings = 1, tx_rings = 1;
    // size of queues
    uint16_t nb_rxd = RX_RING_SIZE;
    uint16_t nb_txd = TX_RING_SIZE;
    // return value for later procedure calls
    int retval;
    // counting variable used for for loop
    uint16_t q;
    // information about NIC, on which pci_slot...
    struct rte_eth_dev_info dev_info;
    // filled later with default value of NIC
    // TODO: research in doku!
    struct rte_eth_txconf txconf;

    // does port exist?
    if (!rte_eth_dev_is_valid_port(port))
        return -1;

    // fill def_info with data
    rte_eth_dev_info_get(port, &dev_info);
    // does queue have offload capacity? Is MBUF offload possible (bool)?
    if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
        // |= is not allowed operator. negation of DEV_TX_OFFL...
        port_conf.txmode.offloads |= DEV_TX_OFFLOAD_MBUF_FAST_FREE;

    /* Configure the Ethernet device. */
    // address NIC; assign queues; 1 queue per port
    retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
    if (retval != 0)
        return retval;

    // nb_rxd ist size of queue (i guess)
    retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
    if (retval != 0)
        return retval;

    /* Allocate and set up 1 RX queue per Ethernet port. */
    // TODO: This may be the key to RSS
    for (q = 0; q < rx_rings; q++) {
        retval = rte_eth_rx_queue_setup(
            port, q, nb_rxd, rte_eth_dev_socket_id(port), NULL, mbuf_pool);
        if (retval < 0)
            return retval;
    }

    // initializes tx config for passing it to the queue setup
    txconf = dev_info.default_txconf;
    txconf.offloads = port_conf.txmode.offloads;
    /* Allocate and set up 1 TX queue per Ethernet port. */
    for (q = 0; q < tx_rings; q++) {
        retval = rte_eth_tx_queue_setup(port, q, nb_txd,
                                        rte_eth_dev_socket_id(port), &txconf);
        if (retval < 0)
            return retval;
    }

    /* Start the Ethernet port. */
    // see doc
    retval = rte_eth_dev_start(port);
    if (retval < 0)
        return retval;

    /* Display the port MAC address. */
    struct rte_ether_addr addr;
    rte_eth_macaddr_get(port, &addr);
    printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8
           " %02" PRIx8 " %02" PRIx8 "\n",
           port, addr.addr_bytes[0], addr.addr_bytes[1], addr.addr_bytes[2],
           addr.addr_bytes[3], addr.addr_bytes[4], addr.addr_bytes[5]);

    /* Enable RX in promiscuous mode for the Ethernet device. */
    // see doc
    // do we need it?
    rte_eth_promiscuous_enable(port);

    return 0;
}

/**
 * @brief reading from input and writing to output
 *
 * This is the main thread that does the work, reading from an input port and
 * writing to an output port.
 */
static __attribute__((noreturn)) void lcore_main(void) {
    // noreturn extension to void
    // TODO: see doc!
    uint16_t port;

    /*
     * Check that the port is on the same NUMA node as the polling thread
     * for best performance.
     */
    RTE_ETH_FOREACH_DEV(port) {
    // Ignore this, if we dont use NUMA
        if (rte_eth_dev_socket_id(port) > 0 &&
            rte_eth_dev_socket_id(port) != (int)rte_socket_id())
            printf("WARNING, port %u is on remote NUMA node to "
                   "polling thread.\n\tPerformance will "
                   "not be optimal.\n",
                   port);
    }

    printf("\nCore %u forwarding packets. [Ctrl+C to quit]\n", rte_lcore_id());

    /* Run until the application is quit or killed. */
    for (;;) {
        /*
         * Receive packets on a port and forward them on the paired
         * port. The mapping is 0 -> 1, 1 -> 0, 2 -> 3, 3 -> 2, etc.
         */
        RTE_ETH_FOREACH_DEV(port) {

            /* Get burst of RX packets, from first port of pair. */
            // poll packets
            struct rte_mbuf* bufs[BURST_SIZE];
            const uint16_t nb_rx = rte_eth_rx_burst(port, 0, bufs, BURST_SIZE);

            if (unlikely(nb_rx == 0))
                continue;

            /* Send burst of TX packets, to second port of pair. */
            const uint16_t nb_tx = rte_eth_tx_burst(port ^ 1, 0, bufs, nb_rx);

            /* Free any unsent packets. */
            if (unlikely(nb_tx < nb_rx)) {
                uint16_t buf;
                for (buf = nb_tx; buf < nb_rx; buf++)
                    rte_pktmbuf_free(bufs[buf]);
            }
        }
    }
}

/**
 * @brief init and call per lcore
 *
 * The main function, which does initialization and calls the per-lcore
 * functions.
 */
int main(int argc, char* argv[]) {
    struct rte_mempool* mbuf_pool;
    unsigned nb_ports;
    uint16_t portid;

    /* Initialize the Environment Abstraction Layer (EAL). */
    int ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

    argc -= ret;
    argv += ret;

    /* Check that there is an even number of ports to send/receive on. */
    nb_ports = rte_eth_dev_count_avail();
    if (nb_ports < 2 || (nb_ports & 1))
        rte_exit(EXIT_FAILURE, "Error: number of ports must be even\n");

    /* Creates a new mempool in memory to hold the mbufs. */
    mbuf_pool = rte_pktmbuf_pool_create(
        "MBUF_POOL", NUM_MBUFS * nb_ports, MBUF_CACHE_SIZE, 0,
        RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    if (mbuf_pool == NULL)
        rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

    /* Initialize all ports. */
    RTE_ETH_FOREACH_DEV(portid) {
        if (port_init(portid, mbuf_pool) != 0)
            rte_exit(EXIT_FAILURE, "Cannot init port %" PRIu16 "\n", portid);
    }

    if (rte_lcore_count() > 1)
        printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");

    /* Call lcore_main on the master core only. */
    lcore_main();

    return 0;
}