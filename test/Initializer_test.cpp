#include <catch2/catch.hpp>
#include <cstdio>
#include <iostream>

#include "ConfigurationManagement/Configurator.hpp"

TEST_CASE("Calculate number of threads", "[]") {
    /*
        Configurator::instance()->read_config("../test/Initializer_config.json");

        uint16_t nb_worker_threads = 0;
        // calculate default value of worker threads
        uint16_t nb_worker_threads_default = 12;

        // get number of threads from config
        std::string nb = Configurator::instance()->get_config_as_string(
            "number_of_worker_threads");

        if (nb == "default") {
    nb_worker_threads = nb_worker_threads_default;
}
else {

    uint16_t nb_int = std::stoi(nb);

    REQUIRE(nb_int == 11);

    if (nb_int > nb_worker_threads_default) {
        throw std::runtime_error(
            "number_of_worker_threads is greater than the maximal possible "
            "number of worker threads. Either s number_of_worker_threads "
            "to 'default' or set it to a valu smaller or equal than " +
            std::to_string(nb_worker_threads_default) + " and bigger than 0.");
    } else if (nb_int <= 0) {
        throw std::runtime_error(
            "number_of_worker_threads is greater than or equal 0. Either "
            " set number_of_worker_threads "
            " to 'default' or set it to a value less or equal than " +
            std::to_string(nb_worker_threads_default) + " and greater than 0.");
    }

    nb_worker_threads = nb_int;
}

REQUIRE(nb_worker_threads <= nb_worker_threads_default);
REQUIRE(nb_worker_threads_default > 0);
REQUIRE(nb_worker_threads_default == 11);
*/
}
