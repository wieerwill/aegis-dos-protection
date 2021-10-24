#pragma once

#include <cstddef>

#include <rte_branch_prediction.h>
#include <rte_common.h>
#include <rte_config.h>
#include <rte_mbuf_core.h>
#include <rte_mempool.h>
#include <stdint.h>

#define __rte_mbuf_sanity_check(m, is_h)                                       \
    do {                                                                       \
    } while (0)

static inline uint16_t rte_pktmbuf_headroom(const struct rte_mbuf* m) {
    __rte_mbuf_sanity_check(m, 0);
    return m->data_off;
}

static inline void rte_pktmbuf_reset_headroom(struct rte_mbuf* m) {
    m->data_off = (uint16_t)RTE_PKTMBUF_HEADROOM;
    //(uint16_t)RTE_MIN((uint16_t)RTE_PKTMBUF_HEADROOM, (uint16_t)m->buf_len);
}

static inline void rte_pktmbuf_reset(struct rte_mbuf* m) {
    m->next = NULL;
    m->pkt_len = 0;
    m->tx_offload = 0;
    m->vlan_tci = 0;
    m->vlan_tci_outer = 0;
    m->nb_segs = 1;
    m->port = UINT16_MAX;

    m->ol_flags &= (1ULL << 61);
    m->packet_type = 0;
    rte_pktmbuf_reset_headroom(m);

    m->data_len = 0;
    __rte_mbuf_sanity_check(m, 1);
}

static struct rte_mbuf* rte_pktmbuf_alloc(struct rte_mempool* mp) {

    struct rte_mbuf* m = NULL;
    m = new struct rte_mbuf;
    if (m != NULL) {
        m->refcnt = 1;
        m->nb_segs = 1;
        m->next = NULL;
        rte_pktmbuf_reset(m);

        m->buf_addr = new uint8_t[2000];
        m->buf_len = 0;
    }

    return m;
}

static inline void rte_pktmbuf_free(struct rte_mbuf* m) {
    struct rte_mbuf* m_next;

    if (m != NULL)
        __rte_mbuf_sanity_check(m, 1);

    delete[] m->buf_addr;
    m->buf_addr = nullptr;

    while (m != NULL) {
        m_next = m->next;
        delete m;
        m = m_next;
    }
}

static inline char* rte_pktmbuf_prepend(struct rte_mbuf* m, uint16_t len) {
    __rte_mbuf_sanity_check(m, 1);

    if (unlikely(len > rte_pktmbuf_headroom(m))) {
        return NULL;
    }

    /* NB: elaborating the subtraction like this instead of using
     *     -= allows us to ensure the result type is uint16_t
     *     avoiding compiler warnings on gcc 8.1 at least */
    m->data_off = static_cast<uint16_t>(m->data_off - len);
    m->data_len = static_cast<uint16_t>(m->data_len + len);
    m->pkt_len = (m->pkt_len + len);

    return (char*)m->buf_addr + m->data_off;
}

static inline uint16_t rte_pktmbuf_tailroom(const struct rte_mbuf* m) {
    __rte_mbuf_sanity_check(m, 0);
    return (uint16_t)(m->buf_len - rte_pktmbuf_headroom(m) - m->data_len);
}

static inline struct rte_mbuf* rte_pktmbuf_lastseg(struct rte_mbuf* m) {
    __rte_mbuf_sanity_check(m, 1);
    while (m->next != NULL)
        m = m->next;
    return m;
}

static inline char* rte_pktmbuf_append(struct rte_mbuf* m, uint16_t len) {
    void* tail;
    struct rte_mbuf* m_last;

    __rte_mbuf_sanity_check(m, 1);

    m_last = rte_pktmbuf_lastseg(m);
    if (unlikely(len > rte_pktmbuf_tailroom(m_last)))
        return NULL;

    tail = (char*)m_last->buf_addr + m_last->data_off + m_last->data_len;
    m_last->data_len = (uint16_t)(m_last->data_len + len);
    m->pkt_len = (m->pkt_len + len);
    return (char*)tail;
}
