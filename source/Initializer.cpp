
#include <rte_eal.h>
#include <rte_lcore.h>

#include <stdexcept>

#include "Configurator.hpp"
#include "Definitions.hpp"

#include "Initializer.hpp"

rte_mempool* Initializer::init_dpdk(int argc, char** argv,
                                    uint16_t& nb_worker_threads) {

    uint8_t hash_key[RSS_HASH_KEY_LENGTH] = {
        0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
        0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
        0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
        0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
    };

    rte_eth_conf port_conf = {
        .rxmode =
            {
                .mq_mode = ETH_MQ_RX_RSS,
                .max_rx_pkt_len = RTE_ETHER_MAX_LEN,
                .offloads =
                    DEV_RX_OFFLOAD_CHECKSUM
            },
        .txmode =
            {
                .offloads =
                    (DEV_TX_OFFLOAD_IPV4_CKSUM | DEV_TX_OFFLOAD_UDP_CKSUM |
                    DEV_TX_OFFLOAD_TCP_CKSUM),
            },  
        .rx_adv_conf = {.rss_conf =
                            {
                                .rss_key = hash_key,
                                .rss_key_len = RSS_HASH_KEY_LENGTH,
                                .rss_hf =
                                    ETH_RSS_IP | ETH_RSS_TCP | ETH_RSS_UDP,
                            }},
    };

    return init_dpdk_template(NUM_NON_WORKER_THREADS, port_conf, argc, argv,
                              nb_worker_threads);
}

rte_mempool* Initializer::init_dpdk_attacker(int argc, char** argv,
                                             uint16_t& nb_worker_threads) {
    rte_eth_conf port_conf = {
        .rxmode =
            {
                .max_rx_pkt_len = RTE_ETHER_MAX_LEN,
            },
        .txmode =
            {
                .offloads =
                    (DEV_TX_OFFLOAD_IPV4_CKSUM | DEV_TX_OFFLOAD_UDP_CKSUM |
                    DEV_TX_OFFLOAD_TCP_CKSUM),
            },  
    };

    return init_dpdk_template(0, port_conf, argc, argv, nb_worker_threads);
}

rte_mempool* Initializer::init_dpdk_template(uint16_t nb_non_worker_threads,
                                             rte_eth_conf port_conf, int argc,
                                             char** argv,
                                             uint16_t& nb_worker_threads) {
    int ret;
    unsigned int nb_ports;
    uint16_t portid; //< init portid
    struct rte_mempool* mbuf_pool;

    // initialize eal
    ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Cannot init EAL\n");
    }

    // initialize number of worker threads
    init_number_threads(nb_non_worker_threads, nb_worker_threads);

    // Check that there is an even number of ports to send/receive on.
    nb_ports = rte_eth_dev_count_avail();
    if (nb_ports < 2 || (nb_ports & 1)) {
        rte_exit(EXIT_FAILURE, "Error: number of ports must be even\n");
    }

    // Creates a new mempool in memory to hold the mbufs.

    // This argument must be lower or equal to RTE_MEMPOOL_CACHE_MAX_SIZE (=
    // 512)
    // and n / 1.5. It is advised to choose cache_size to have "n modulo
    // cache_size
    // == 0". size from basicfwd program in dpdk examples: 250
    unsigned cache_size = RTE_MEMPOOL_CACHE_MAX_SIZE;
    if (cache_size > NUM_MBUF_POOL_ELEMENTS / 1.5) {
        cache_size = unsigned(NUM_MBUF_POOL_ELEMENTS / 1.5);
    }

    while (NUM_MBUF_POOL_ELEMENTS % cache_size != 0 || cache_size == 0) {
        cache_size -= 1;
    }

    mbuf_pool = rte_pktmbuf_pool_create(
        "MBUF_POOL", NUM_MBUF_POOL_ELEMENTS, 0 /*cache_size*/, 0,
        RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    if (mbuf_pool == nullptr) {
        rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");
    }

    // Initialize all ports.
    RTE_ETH_FOREACH_DEV(portid) {
        try {
            init_port(port_conf, portid, mbuf_pool, nb_worker_threads);
            }
        catch (std::exception& e) {
            rte_exit(EXIT_FAILURE, "Cannot init port %" PRIu16 "\n", portid);
        }
    }

    return mbuf_pool;
}

void Initializer::init_port(rte_eth_conf port_conf, uint16_t port,
                            struct rte_mempool* mbuf_pool,
                            uint16_t nb_worker_threads) {
    // the "port" is a port_id which is fetched in the main-function by the
    // Makro RTE_ETH_FOREACH_DEV The mbuf_pool is needed for queue
    // initialization

    const uint16_t rx_rings =
        nb_worker_threads; // rte_lcore_count() many worker threads with an
                           // rx queue on each port
    const uint16_t tx_rings = rx_rings; // number of rx and tx queues
    uint16_t nb_rxd = RX_RING_SIZE;     // size of queues
    uint16_t nb_txd = TX_RING_SIZE;
    int retval; // return value for later procedure calls
    uint16_t q; // counting variable used for for loop
    struct rte_eth_dev_info
        dev_info;                 // information about NIC, on which pci_slot...
    struct rte_eth_txconf txconf; // filled later with default value of NIC

    // test if port exists
    if (!rte_eth_dev_is_valid_port(port))
        throw std::exception();

    // fill def_info with data
    rte_eth_dev_info_get(port, &dev_info);
    // test if queue has offload capacity; if MBUF offload is possible
    if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
        // |= is not allowed operator. negation of DEV_TX_OFFL...
        port_conf.txmode.offloads |= DEV_TX_OFFLOAD_MBUF_FAST_FREE;
    // test if checksum offloading is possible
    if (((dev_info.tx_offload_capa & DEV_TX_OFFLOAD_TCP_CKSUM) == DEV_TX_OFFLOAD_TCP_CKSUM) && 
            ((dev_info.tx_offload_capa & DEV_TX_OFFLOAD_IPV4_CKSUM) == DEV_TX_OFFLOAD_IPV4_CKSUM) ){
        std::cout << "ethernet device is checksum offloading capabel" << std::endl;
    } else {
        std::cout << "ethernet device is not checksum offloading capabel" << std::endl;
    }
    if (((dev_info.tx_queue_offload_capa & DEV_TX_OFFLOAD_IPV4_CKSUM) == DEV_TX_OFFLOAD_IPV4_CKSUM) &&
            ((dev_info.tx_queue_offload_capa & DEV_TX_OFFLOAD_TCP_CKSUM) == DEV_TX_OFFLOAD_TCP_CKSUM)) {
        std::cout << "queue is checksum offloading capabel" << std::endl;
    } else {
        std::cout << "queue is not checksum offloading capabel" << std::endl;
    }

    // Configure the Ethernet device.
    // address NIC; assign queues; 1 queue per port
    retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
    if (retval != 0)
        throw std::exception();

    retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
    if (retval != 0)
        throw std::exception();

    // Allocate and set up RX queues per Ethernet port.
    for (q = 0; q < rx_rings; q++) {
        retval = rte_eth_rx_queue_setup(
            port, q, nb_rxd, rte_eth_dev_socket_id(port), NULL, mbuf_pool);
        if (retval < 0)
            throw std::exception();
    }

    // initializes tx config for passing it to the queue setup
    txconf = dev_info.default_txconf;
    txconf.offloads = port_conf.txmode.offloads;
    // Allocate and set up 1 TX queue per Ethernet port.
    for (q = 0; q < tx_rings; q++) {
        retval = rte_eth_tx_queue_setup(port, q, nb_txd,
                                        rte_eth_dev_socket_id(port), &txconf);
        if (retval < 0)
            throw std::exception();
    }

    // Start the Ethernet port.
    retval = rte_eth_dev_start(port);
    if (retval < 0)
        throw std::exception();

    // Display the port MAC address.
    struct rte_ether_addr addr;
    rte_eth_macaddr_get(port, &addr);
    printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8
           " %02" PRIx8 " %02" PRIx8 "\n",
           port, addr.addr_bytes[0], addr.addr_bytes[1], addr.addr_bytes[2],
           addr.addr_bytes[3], addr.addr_bytes[4], addr.addr_bytes[5]);

    // Enable RX in promiscuous mode for the Ethernet device.
    // TODO: do we need it
    rte_eth_promiscuous_enable(port);
}

void Initializer::init_number_threads(uint16_t nb_non_worker_threads,
                                      uint16_t& nb_worker_threads) {
    // calculate default value of worker threads
    uint16_t nb_worker_threads_default =
        rte_lcore_count() - nb_non_worker_threads;

    // check if value exists or if it has to be calculated
    if (Configurator::instance()->entry_exists("number_of_worker_threads")) {
        std::string nb = Configurator::instance()->get_config_as_string(
            "number_of_worker_threads");

        uint16_t nb_int = 0;

        try { /* value is convertable to an integer */
            nb_int = std::stoi(nb);
        } catch (
            std::exception& e) { /* value is not convertable to an integer */
            throw std::runtime_error(
                "The given value at key 'number_of_worker_threads' in "
                "config.json is not "
                "convertable to an integer value. Set "
                "number_of_worker_threads "
                "either to 'default' or to a value less or equal than " +
                std::to_string(nb_worker_threads_default) +
                " and bigger than 0.");
        }

        if (nb_int > nb_worker_threads_default) {
            throw std::runtime_error(
                "The given value at key 'number_of_worker_threads' in "
                "config.json is out of bounds. Set "
                "number_of_worker_threads "
                "either to 'default' or to a value smaller or equal than " +
                std::to_string(nb_worker_threads_default) +
                " and bigger than 0.");
        } else if (nb_int <= 0) {
            throw std::runtime_error(
                "The given value at key 'number_of_worker_threads' in "
                "config.json is out of bounds. "
                "Either "
                "set number_of_worker_threads "
                "to 'default' or set it to a value less or equal than " +
                std::to_string(nb_worker_threads_default) +
                " and greater than 0.");
        }

        nb_worker_threads = nb_int;

        LOG_INFO << "Value " << nb_int
                 << " for number_of_worker_threads found in config.json. Using "
                 << nb_int << " worker threads and " << nb_non_worker_threads
                 << " non worker thread." << LOG_END;
    } else {
        nb_worker_threads = nb_worker_threads_default;
        LOG_INFO << "No value for number_of_worker_threads found in "
                    "config_attacker.json. Using "
                 << nb_worker_threads << " worker threads and "
                 << nb_non_worker_threads << " non worker thread." << LOG_END;
    }
}
