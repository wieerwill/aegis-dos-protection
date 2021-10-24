#pragma once

#include <rte_mbuf.h>
#include <stdint.h>

struct rte_eth_conf {
    /*
    uint32_t link_speeds;
    struct rte_eth_rxmode rxmode;
    struct rte_eth_txmode txmode;
    uint32_t lpbk_mode;
    struct {
        struct rte_eth_rss_conf rss_conf;
        struct rte_eth_vmdq_dcb_conf vmdq_dcb_conf;
        struct rte_eth_dcb_rx_conf dcb_rx_conf;
        struct rte_eth_vmdq_rx_conf vmdq_rx_conf;
    } rx_adv_conf;
    union {
        struct rte_eth_vmdq_dcb_tx_conf vmdq_dcb_tx_conf;
        struct rte_eth_dcb_tx_conf dcb_tx_conf;
        struct rte_eth_vmdq_tx_conf vmdq_tx_conf;
    } tx_adv_conf;
    uint32_t dcb_capability_en;
    struct rte_fdir_conf fdir_conf;
    struct rte_intr_conf intr_conf;
    */
};

static inline uint16_t rte_eth_tx_burst(uint16_t port_id, uint16_t queue_id,
                                        struct rte_mbuf** tx_pkts,
                                        uint16_t nb_pkts) {
    for (int i = 0; i < nb_pkts; ++i) {
        rte_pktmbuf_free(tx_pkts[i]);
    }
}

static inline uint16_t rte_eth_rx_burst(uint16_t port_id, uint16_t queue_id,
                                        struct rte_mbuf** rx_pkts,
                                        const uint16_t nb_pkts) {
    rte_mempool* mp;
    for (int i = 0; i < nb_pkts; ++i) {
        rx_pkts[i] = rte_pktmbuf_alloc(mp);
    }
}