/**
 * @file Initializer.hpp
 * @author Robert
 * @brief 
 * @version 0.1
 * @date 2021-07-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <rte_ethdev.h>
#include <rte_mempool.h>

#include <stdint.h>

class Initializer {
  public:
    /**
     * @brief Initializes everything regarding DPDK and configures the ports.
     *
     * Especially: Initializes the variable nb_worker_threads
     *
     * @param[in] argc
     * @param[in] argv
     * @param[out] nb_worker_threads
     */
    rte_mempool* init_dpdk(int argc, char** argv, uint16_t& nb_worker_threads);

    rte_mempool* init_dpdk_attacker(int argc, char** argv,
                                    uint16_t& nb_worker_threads);

  private:
    /**
     * @brief init NIC
     *
     * get the NIC by port identifier and bind it to dpdk with given mempool.
     * Setting up TX/RX queues per ethernet port. Then starting the device and
     * printing MAC address
     *
     * @param port_conf
     * @param port set port identifier to use
     * @param mbuf_pool set mempool of dpdk to use
     * @param[in] nb_worker_threads number of worker threads
     * @return 0 or error code
     * 
     * \todo describe port_conf
     */
    void init_port(rte_eth_conf port_conf, uint16_t port,
                   struct rte_mempool* mbuf_pool, uint16_t nb_worker_threads);

    /**
     * @brief initializes number of threads
     *
     * @param nb_non_worker_threads
     * @param[out] nb_worker_threads
     * 
     * \todo nb_non_worker_threads description
     */
    void init_number_threads(uint16_t nb_non_worker_threads,
                             uint16_t& nb_worker_threads);

    rte_mempool* init_dpdk_template(uint16_t nb_non_worker_threads,
                                    rte_eth_conf port_conf, int argc,
                                    char** argv, uint16_t& nb_worker_threads);
};
