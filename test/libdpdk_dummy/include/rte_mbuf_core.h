#pragma once

#include <rte_common.h>
#include <stdint.h>

#define rte_pktmbuf_mtod_offset(m, t, o)                                       \
    ((t)((char*)(m)->buf_addr + (m)->data_off + (o)))

#define rte_pktmbuf_mtod(m, t) rte_pktmbuf_mtod_offset(m, t, 0)

#define PKT_RX_VLAN (1ULL << 0)

#define PKT_RX_RSS_HASH (1ULL << 1)

#define PKT_RX_FDIR (1ULL << 2)

#define PKT_RX_L4_CKSUM_BAD (1ULL << 3)

#define PKT_RX_IP_CKSUM_BAD (1ULL << 4)

#define PKT_RX_OUTER_IP_CKSUM_BAD (1ULL << 5)

#define PKT_RX_EIP_CKSUM_BAD                                                   \
    RTE_DEPRECATED(PKT_RX_EIP_CKSUM_BAD) PKT_RX_OUTER_IP_CKSUM_BAD

#define PKT_RX_VLAN_STRIPPED (1ULL << 6)

#define PKT_RX_IP_CKSUM_MASK ((1ULL << 4) | (1ULL << 7))

#define PKT_RX_IP_CKSUM_UNKNOWN 0
#define PKT_RX_IP_CKSUM_BAD (1ULL << 4)
#define PKT_RX_IP_CKSUM_GOOD (1ULL << 7)
#define PKT_RX_IP_CKSUM_NONE ((1ULL << 4) | (1ULL << 7))

#define PKT_RX_L4_CKSUM_MASK ((1ULL << 3) | (1ULL << 8))

#define PKT_RX_L4_CKSUM_UNKNOWN 0
#define PKT_RX_L4_CKSUM_BAD (1ULL << 3)
#define PKT_RX_L4_CKSUM_GOOD (1ULL << 8)
#define PKT_RX_L4_CKSUM_NONE ((1ULL << 3) | (1ULL << 8))

#define PKT_RX_IEEE1588_PTP (1ULL << 9)

#define PKT_RX_IEEE1588_TMST (1ULL << 10)

#define PKT_RX_FDIR_ID (1ULL << 13)

#define PKT_RX_FDIR_FLX (1ULL << 14)

#define PKT_RX_QINQ_STRIPPED (1ULL << 15)

#define PKT_RX_LRO (1ULL << 16)

/* There is no flag defined at offset 17. It is free for any future use. */

#define PKT_RX_SEC_OFFLOAD (1ULL << 18)

#define PKT_RX_SEC_OFFLOAD_FAILED (1ULL << 19)

#define PKT_RX_QINQ (1ULL << 20)

#define PKT_RX_OUTER_L4_CKSUM_MASK ((1ULL << 21) | (1ULL << 22))

#define PKT_RX_OUTER_L4_CKSUM_UNKNOWN 0
#define PKT_RX_OUTER_L4_CKSUM_BAD (1ULL << 21)
#define PKT_RX_OUTER_L4_CKSUM_GOOD (1ULL << 22)
#define PKT_RX_OUTER_L4_CKSUM_INVALID ((1ULL << 21) | (1ULL << 22))

/* add new RX flags here, don't forget to update PKT_FIRST_FREE */

#define PKT_FIRST_FREE (1ULL << 23)
#define PKT_LAST_FREE (1ULL << 40)

/* add new TX flags here, don't forget to update PKT_LAST_FREE  */

#define PKT_TX_OUTER_UDP_CKSUM (1ULL << 41)

#define PKT_TX_UDP_SEG (1ULL << 42)

#define PKT_TX_SEC_OFFLOAD (1ULL << 43)

#define PKT_TX_MACSEC (1ULL << 44)

#define PKT_TX_TUNNEL_VXLAN (0x1ULL << 45)
#define PKT_TX_TUNNEL_GRE (0x2ULL << 45)
#define PKT_TX_TUNNEL_IPIP (0x3ULL << 45)
#define PKT_TX_TUNNEL_GENEVE (0x4ULL << 45)

#define PKT_TX_TUNNEL_MPLSINUDP (0x5ULL << 45)
#define PKT_TX_TUNNEL_VXLAN_GPE (0x6ULL << 45)
#define PKT_TX_TUNNEL_GTP (0x7ULL << 45)

#define PKT_TX_TUNNEL_IP (0xDULL << 45)

#define PKT_TX_TUNNEL_UDP (0xEULL << 45)
/* add new TX TUNNEL type here */
#define PKT_TX_TUNNEL_MASK (0xFULL << 45)

#define PKT_TX_QINQ (1ULL << 49)

#define PKT_TX_QINQ_PKT PKT_TX_QINQ

#define PKT_TX_TCP_SEG (1ULL << 50)

#define PKT_TX_IEEE1588_TMST (1ULL << 51)

#define PKT_TX_L4_NO_CKSUM (0ULL << 52)
#define PKT_TX_TCP_CKSUM (1ULL << 52)

#define PKT_TX_SCTP_CKSUM (2ULL << 52)

#define PKT_TX_UDP_CKSUM (3ULL << 52)

#define PKT_TX_L4_MASK (3ULL << 52)

#define PKT_TX_IP_CKSUM (1ULL << 54)

#define PKT_TX_IPV4 (1ULL << 55)

#define PKT_TX_IPV6 (1ULL << 56)

#define PKT_TX_VLAN (1ULL << 57)
/* this old name is deprecated */
#define PKT_TX_VLAN_PKT PKT_TX_VLAN

#define PKT_TX_OUTER_IP_CKSUM (1ULL << 58)

#define PKT_TX_OUTER_IPV4 (1ULL << 59)

#define PKT_TX_OUTER_IPV6 (1ULL << 60)

enum {
    RTE_MBUF_L2_LEN_BITS = 7,
    RTE_MBUF_L3_LEN_BITS = 9,
    RTE_MBUF_L4_LEN_BITS = 8,
    RTE_MBUF_TSO_SEGSZ_BITS = 16,
    RTE_MBUF_OUTL3_LEN_BITS = 9,
    RTE_MBUF_OUTL2_LEN_BITS = 7
};

struct rte_mbuf {
    void* buf_addr;
    uint16_t buf_len;

    struct rte_mbuf* next;
    uint32_t pkt_len;
    uint16_t vlan_tci;
    uint16_t vlan_tci_outer;
    uint16_t nb_segs;
    uint16_t port;
    uint64_t ol_flags;
    uint32_t packet_type;
    uint16_t data_len;
    uint16_t data_off;
    uint16_t refcnt;

    /* fields to support TX offloads */
    RTE_STD_C11
    union {
        uint64_t tx_offload;
        __extension__ struct {
            uint64_t l2_len : RTE_MBUF_L2_LEN_BITS;
            uint64_t l3_len : RTE_MBUF_L3_LEN_BITS;
            uint64_t l4_len : RTE_MBUF_L4_LEN_BITS;
            uint64_t tso_segsz : RTE_MBUF_TSO_SEGSZ_BITS;
            /*
             * Fields for Tx offloading of tunnels.
             * These are undefined for packets which don't request
             * any tunnel offloads (outer IP or UDP checksum,
             * tunnel TSO).
             *
             * PMDs should not use these fields unconditionally
             * when calculating offsets.
             *
             * Applications are expected to set appropriate tunnel
             * offload flags when they fill in these fields.
             */
            uint64_t outer_l3_len : RTE_MBUF_OUTL3_LEN_BITS;
            uint64_t outer_l2_len : RTE_MBUF_OUTL2_LEN_BITS;
            /* uint64_t unused:RTE_MBUF_TXOFLD_UNUSED_BITS; */
        };
    };
};
