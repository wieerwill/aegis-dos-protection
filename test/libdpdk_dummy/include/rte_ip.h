#pragma once

#include <netinet/in.h>
#include <rte_byteorder.h>
#include <rte_mbuf_core.h>
#include <stdint.h>

#define RTE_IPV4_HDR_IHL_MASK (0x0f)
#define RTE_IPV4_IHL_MULTIPLIER (4)
#define IPPROTO_UDP IPPROTO_UDP

struct rte_ipv4_hdr {
    uint8_t version_ihl;
    uint8_t type_of_service;
    rte_be16_t total_length;
    rte_be16_t packet_id;
    rte_be16_t fragment_offset;
    uint8_t time_to_live;
    uint8_t next_proto_id;
    rte_be16_t hdr_checksum;
    rte_be32_t src_addr;
    rte_be32_t dst_addr;
} __attribute__((__packed__));

struct rte_ipv6_hdr {
    rte_be32_t vtc_flow;
    rte_be16_t payload_len;
    uint8_t proto;
    uint8_t hop_limits;
    uint8_t src_addr[16];
    uint8_t dst_addr[16];
} __attribute__((__packed__));

static inline uint8_t rte_ipv4_hdr_len(const struct rte_ipv4_hdr* ipv4_hdr) {
    return (uint8_t)((ipv4_hdr->version_ihl & RTE_IPV4_HDR_IHL_MASK) *
                     RTE_IPV4_IHL_MULTIPLIER);
}

static inline uint16_t __rte_raw_cksum_reduce(uint32_t sum) {
    sum = ((sum & 0xffff0000) >> 16) + (sum & 0xffff);
    sum = ((sum & 0xffff0000) >> 16) + (sum & 0xffff);
    return (uint16_t)sum;
}

static inline uint32_t __rte_raw_cksum(const void* buf, size_t len,
                                       uint32_t sum) {
    /* workaround gcc strict-aliasing warning */
    uintptr_t ptr = (uintptr_t)buf;
    typedef uint16_t __attribute__((__may_alias__)) u16_p;
    const u16_p* u16_buf = (const u16_p*)ptr;

    while (len >= (sizeof(*u16_buf) * 4)) {
        sum += u16_buf[0];
        sum += u16_buf[1];
        sum += u16_buf[2];
        sum += u16_buf[3];
        len -= sizeof(*u16_buf) * 4;
        u16_buf += 4;
    }
    while (len >= sizeof(*u16_buf)) {
        sum += *u16_buf;
        len -= sizeof(*u16_buf);
        u16_buf += 1;
    }

    /* if length is in odd bytes */
    if (len == 1)
        sum += *((const uint8_t*)u16_buf);

    return sum;
}

static inline uint16_t rte_raw_cksum(const void* buf, size_t len) {
    uint32_t sum;

    sum = __rte_raw_cksum(buf, len, 0);
    return __rte_raw_cksum_reduce(sum);
}

static inline uint16_t rte_ipv4_cksum(const struct rte_ipv4_hdr* ipv4_hdr) {
    uint16_t cksum;
    cksum = rte_raw_cksum(ipv4_hdr, rte_ipv4_hdr_len(ipv4_hdr));
    return (uint16_t)~cksum;
}

static inline uint16_t rte_ipv4_phdr_cksum(const struct rte_ipv4_hdr* ipv4_hdr,
                                           uint64_t ol_flags) {
    struct ipv4_psd_header {
        uint32_t src_addr; /* IP address of source host. */
        uint32_t dst_addr; /* IP address of destination host. */
        uint8_t zero;      /* zero. */
        uint8_t proto;     /* L4 protocol type. */
        uint16_t len;      /* L4 length. */
    } psd_hdr;

    psd_hdr.src_addr = ipv4_hdr->src_addr;
    psd_hdr.dst_addr = ipv4_hdr->dst_addr;
    psd_hdr.zero = 0;
    psd_hdr.proto = ipv4_hdr->next_proto_id;

    if (ol_flags & PKT_TX_TCP_SEG) {
        psd_hdr.len = 0;
    } else {
        psd_hdr.len = rte_cpu_to_be_16(
            (uint16_t)(rte_be_to_cpu_16(ipv4_hdr->total_length) -
                       sizeof(struct rte_ipv4_hdr)));
    }

    return rte_raw_cksum(&psd_hdr, sizeof(psd_hdr));
}

static inline uint16_t
rte_ipv4_udptcp_cksum(const struct rte_ipv4_hdr* ipv4_hdr, const void* l4_hdr) {
    uint32_t cksum;
    uint32_t l3_len, l4_len;
    uint8_t ip_hdr_len;

    ip_hdr_len = rte_ipv4_hdr_len(ipv4_hdr);
    l3_len = rte_be_to_cpu_16(ipv4_hdr->total_length);
    if (l3_len < ip_hdr_len)
        return 0;

    l4_len = l3_len - ip_hdr_len;

    cksum = rte_raw_cksum(l4_hdr, l4_len);
    cksum += rte_ipv4_phdr_cksum(ipv4_hdr, 0);

    cksum = ((cksum & 0xffff0000) >> 16) + (cksum & 0xffff);
    cksum = (~cksum) & 0xffff;
    /*
     * Per RFC 768:If the computed checksum is zero for UDP,
     * it is transmitted as all ones
     * (the equivalent in one's complement arithmetic).
     */
    if (cksum == 0 && ipv4_hdr->next_proto_id == IPPROTO_UDP)
        cksum = 0xffff;

    return (uint16_t)cksum;
}