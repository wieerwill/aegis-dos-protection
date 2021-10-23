#include "ConfigurationManagement/Configurator.hpp"
#include <catch2/catch.hpp>
#include <cstdio>
#include <iostream>
#include <string>

TEST_CASE("Json Datei einlesen", "[]") {

    REQUIRE_NOTHROW(Configurator::instance()->read_config(
        "../test/ConfigurationManagement/config_test.json"));

    REQUIRE(Configurator::instance()->get_config_as_bool("BOOLEAN") == true);
    REQUIRE(Configurator::instance()->get_config_as_unsigned_int(
                "UNSIGNED_INT") == 42);
    REQUIRE(Configurator::instance()->get_config_as_string("STRING") ==
            "Hello World.");
    REQUIRE(Configurator::instance()->get_config_as_float("FLOAT") == 1.337f);
    REQUIRE(Configurator::instance()->get_config_as_double("DOUBLE") == -3.001);
}

TEST_CASE("nicht existierende Json-Datei", "[]") {
    REQUIRE_THROWS(Configurator::instance()->read_config("non-existent.json"));
    REQUIRE_THROWS(Configurator::instance()->read_config("non-existent.json",
                                                         "typo.json"));
}

TEST_CASE("Boost-Beispiel") {
    LOG_INFO << "Dies ist eine Info Message" << LOG_END;
    LOG_WARNING << "Dies ist eine Warn-Nachricht" << LOG_END;
}

TEST_CASE("Entry does (not) exist") {
    REQUIRE_NOTHROW(Configurator::instance()->read_config(
        "../test/ConfigurationManagement/config_test.json",
        "../test/ConfigurationManagement/default_config_test.json"));

    REQUIRE(Configurator::instance()->entry_exists("fhk4bhf1mx0f") == false);
    REQUIRE(Configurator::instance()->entry_exists("STRING") == true);
    REQUIRE(Configurator::instance()->entry_exists("X") == true);
}

TEST_CASE("Default Config") {
    REQUIRE_NOTHROW(Configurator::instance()->read_config(
        "../test/ConfigurationManagement/config_test.json",
        "../test/ConfigurationManagement/default_config_test.json"));

    REQUIRE(Configurator::instance()->get_config_as_unsigned_int(
                "UNSIGNED_INT") == 42);
    REQUIRE(Configurator::instance()->get_config_as_unsigned_int("UNSIGNED_INT",
                                                                 true) == 666);
    REQUIRE(Configurator::instance()->get_config_as_string("X") == "80085");
}