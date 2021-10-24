#include <stdint.h>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <pthread.h>
#include <string.h>

#define RX_RING_SIZE 128
#define TX_RING_SIZE 512
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 12

/**
 * Ethernet header: Contains the destination address, source address
 * and frame type.
 */
struct ether_hdr
{
    struct ether_addr d_addr; /**< Destination address. */
    struct ether_addr s_addr; /**< Source address. */
    uint16_t ether_type;      /**< Frame type. */
} __attribute__((__packed__));

/**
 * IPv4 Header
 */
struct ipv4_hdr
{
    uint8_t version_ihl;      /**< version and header length */
    uint8_t type_of_service;  /**< type of service */
    uint16_t total_length;    /**< length of packet */
    uint16_t packet_id;       /**< packet ID */
    uint16_t fragment_offset; /**< fragmentation offset */
    uint8_t time_to_live;     /**< time to live */
    uint8_t next_proto_id;    /**< protocol ID */
    uint16_t hdr_checksum;    /**< header checksum */
    uint32_t src_addr;        /**< source address */
    uint32_t dst_addr;        /**< destination address */
} __attribute__((__packed__));

/**
 * UDP Header
 */
struct udp_hdr
{
    uint16_t src_port;    /**< UDP source port. */
    uint16_t dst_port;    /**< UDP destination port. */
    uint16_t dgram_len;   /**< UDP datagram length */
    uint16_t dgram_cksum; /**< UDP datagram checksum */
} __attribute__((__packed__));

static const struct rte_eth_conf port_conf_default = {.rxmode = {.max_rx_pkt_len = ETHER_MAX_LEN}};

static inline int port_init(struct rte_mempool *mbuf_pool)
{
    struct rte_eth_conf port_conf = port_conf_default;
    const uint16_t rx_rings = 1, tx_rings = 1;
    int retval;
    uint16_t q;

    retval = rte_eth_dev_configure(0, rx_rings, tx_rings, &port_conf);
    if (retval != 0)
        return retval;

    /* Allocate and set up 1 RX queue per Ethernet port. */
    for (q = 0; q < rx_rings; q++)
    {
        retval = rte_eth_rx_queue_setup(0, q, RX_RING_SIZE, rte_eth_dev_socket_id(0), NULL, mbuf_pool);
        if (retval < 0)
            return retval;
    }

    /* Allocate and set up 1 TX queue per Ethernet port. */
    for (q = 0; q < tx_rings; q++)
    {
        retval = rte_eth_tx_queue_setup(0, q, TX_RING_SIZE, rte_eth_dev_socket_id(0), NULL);
        if (retval < 0)
            return retval;
    }

    /* Start the Ethernet port. */
    retval = rte_eth_dev_start(0);
    if (retval < 0)
        return retval;

    return 0;
}

int main(int argc, char *argv[])
{
    struct rte_mempool *mbuf_pool;

    int ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "initlize fail!");

    argc -= ret;
    argv += ret;

    /* Creates a new mempool in memory to hold the mbufs. */
    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS, MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    if (mbuf_pool == NULL)
        rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

    /* Initialize all ports. */
    if (port_init(mbuf_pool) != 0)
        rte_exit(EXIT_FAILURE, "Cannot init port %" PRIu8 "\n", 0);

    struct Message
    {
        char data[10];
    };
    struct ether_hdr *eth_hdr;
    struct Message obj = {{'H', 'e', 'l', 'l', 'o', '2', '0', '1', '8'}};
    struct Message *msg;
    struct ether_addr s_addr = {{0x14, 0x02, 0xEC, 0x89, 0x8D, 0x24}};
    struct ether_addr d_addr = {{0x14, 0x02, 0xEC, 0x89, 0xED, 0x54}};
    uint16_t ether_type = 0x0a00;

    struct rte_mbuf *pkt[BURST_SIZE];
    int i;
    for (i = 0; i < BURST_SIZE; i++)
    {
        pkt[i] = rte_pktmbuf_alloc(mbuf_pool);
        eth_hdr = rte_pktmbuf_mtod(pkt[i], struct ether_hdr *);
        eth_hdr->d_addr = d_addr;
        eth_hdr->s_addr = s_addr;
        eth_hdr->ether_type = ether_type;
        msg = (struct Message *)(rte_pktmbuf_mtod(pkt[i], char *) + sizeof(struct ether_hdr));
        *msg = obj;
        int pkt_size = sizeof(struct Message) + sizeof(struct ether_hdr);
        pkt[i]->data_len = pkt_size;
        pkt[i]->pkt_len = pkt_size;
    }

    uint16_t nb_tx = rte_eth_tx_burst(0, 0, pkt, BURST_SIZE);
    printf("nb_tx: ", nb_tx);

    for (i = 0; i < BURST_SIZE; i++)
        rte_pktmbuf_free(pkt[i]);

    return 0;
}
