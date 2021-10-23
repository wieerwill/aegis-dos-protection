#define BOOST_LOG_DYN_LINK 1

#include "Treatment/Treatment.hpp"
#include <catch2/catch.hpp>
#include <chrono>
#include <iostream>
#include <string>
#include <time.h>
#include <unordered_map>

class Treatment_friend {

    Treatment* treatment = new Treatment();

  public:
    static void s_increment_timestamp() { Treatment::s_increment_timestamp(); }

    void treat_packets_to_inside(PacketInfoIpv4Tcp* pkt) {
        treatment->treat_packets_to_inside(pkt);
    }
    void treat_packets_to_outside(PacketInfoIpv4Tcp* pkt) {
        treatment->treat_packets_to_outside(pkt);
    }

    u_int32_t calc_cookie_hash(u_int8_t _s_timestamp, u_int32_t _extip,
                               u_int32_t _intip, u_int16_t _extport,
                               u_int16_t _intport) {
        return treatment->calc_cookie_hash(_s_timestamp, _extip, _intip,
                                           _extport, _intport);
    }

    bool check_syn_cookie(u_int32_t cookie_value, const Data& d) {
        return treatment->check_syn_cookie(cookie_value, d);
    }

    // Getter

    u_int8_t get_s_timestamp() { return treatment->_s_timestamp; }

    u_int64_t get_cookie_secret() { return treatment->_cookie_secret; }

    /*/MbufContainerReceiving* get_packet_to_inside() {
        return treatment->_packet_to_inside;
    }

    MbufContainerReceiving* get_packet_to_outside() {
        return treatment->_packet_to_outside;
    } */

    google::dense_hash_map<Data, Info, TreatmentHash> get_densemap() {
        return treatment->_densemap;
    }

    // Setter

    void set_s_timestamp(u_int8_t value) { treatment->_s_timestamp = value; }
};

TEST_CASE("Hashfunction", "[]") {
    SECTION("XXH3: Not 0", "[]") {
        u_int32_t extip = 12345;
        u_int32_t intip = 98765;
        u_int16_t extport = 123;
        u_int16_t intport = 987;
        XXH64_hash_t hash = XXH3_64bits_withSeed(
            &extip, sizeof(extip),
            XXH3_64bits_withSeed(
                &intip, sizeof(intip),
                XXH3_64bits_withSeed(
                    &extport, sizeof(extport),
                    XXH3_64bits_withSeed(&intport, sizeof(intport), 0))));
        REQUIRE(hash != 0);
    }

    SECTION("XXH3: Backwards-not-Equal", "[]") {
        u_int32_t extip = 12345;
        u_int32_t intip = 98765;
        u_int16_t extport = 123;
        u_int16_t intport = 987;
        XXH64_hash_t forhash = XXH3_64bits_withSeed(
            &extip, sizeof(extip),
            XXH3_64bits_withSeed(
                &intip, sizeof(intip),
                XXH3_64bits_withSeed(
                    &extport, sizeof(extport),
                    XXH3_64bits_withSeed(&intport, sizeof(intport), 0))));
        XXH64_hash_t backhash = XXH3_64bits_withSeed(
            &intip, sizeof(intip),
            XXH3_64bits_withSeed(
                &extip, sizeof(extip),
                XXH3_64bits_withSeed(
                    &intport, sizeof(intport),
                    XXH3_64bits_withSeed(&extport, sizeof(extport), 0))));
        REQUIRE(forhash != backhash);
    }

    SECTION("XXH3: Same data results in same hash values", "[]") {
        u_int32_t extip = 12345;
        u_int32_t intip = 98765;
        u_int16_t extport = 80;
        u_int16_t intport = 21;
        XXH64_hash_t hash1 = XXH3_64bits_withSeed(
            &extip, sizeof(extip),
            XXH3_64bits_withSeed(
                &intip, sizeof(intip),
                XXH3_64bits_withSeed(
                    &extport, sizeof(extport),
                    XXH3_64bits_withSeed(&intport, sizeof(intport), 0))));
        XXH64_hash_t hash2 = XXH3_64bits_withSeed(
            &extip, sizeof(extip),
            XXH3_64bits_withSeed(
                &intip, sizeof(intip),
                XXH3_64bits_withSeed(
                    &extport, sizeof(extport),
                    XXH3_64bits_withSeed(&intport, sizeof(intport), 0))));
        REQUIRE(hash1 == hash2);
    }

    SECTION("XXH3: MAX U_INT_32_T in one ip value") {
        u_int32_t extip = 4294967296 - 1;
        u_int32_t intip = 98765;
        u_int16_t extport = 80;
        u_int16_t intport = 21;
        XXH64_hash_t hash = XXH3_64bits_withSeed(
            &extip, sizeof(extip),
            XXH3_64bits_withSeed(
                &intip, sizeof(intip),
                XXH3_64bits_withSeed(
                    &extport, sizeof(extport),
                    XXH3_64bits_withSeed(&intport, sizeof(intport), 0))));
        REQUIRE(hash != 0);
    }

    SECTION("XXH3: MAX U_INT_16_T in one port value") {
        u_int32_t extip = 12345;
        u_int32_t intip = 98765;
        u_int16_t extport = 65536 - 1;
        u_int16_t intport = 21;
        XXH64_hash_t hash = XXH3_64bits_withSeed(
            &extip, sizeof(extip),
            XXH3_64bits_withSeed(
                &intip, sizeof(intip),
                XXH3_64bits_withSeed(
                    &extport, sizeof(extport),
                    XXH3_64bits_withSeed(&intport, sizeof(intport), 0))));
        REQUIRE(hash != 0);
    }

    SECTION("XXH3: MAX values") {
        u_int32_t extip = 4294967296 - 1;
        u_int32_t intip = 4294967296 - 1;
        u_int16_t extport = 65536 - 1;
        u_int16_t intport = 65536 - 1;
        XXH64_hash_t hash = XXH3_64bits_withSeed(
            &extip, sizeof(extip),
            XXH3_64bits_withSeed(
                &intip, sizeof(intip),
                XXH3_64bits_withSeed(
                    &extport, sizeof(extport),
                    XXH3_64bits_withSeed(&intport, sizeof(intport), 0))));
        REQUIRE(hash != 0);
    }

    SECTION("XXH3: Try wrap around") {
        u_int32_t extip = 4294967296 + 1;
        u_int32_t intip = 4294967296 + 1;
        u_int16_t extport = 65536 + 1;
        u_int16_t intport = 65536 + 1;
        XXH64_hash_t hash = XXH3_64bits_withSeed(
            &extip, sizeof(extip),
            XXH3_64bits_withSeed(
                &intip, sizeof(intip),
                XXH3_64bits_withSeed(
                    &extport, sizeof(extport),
                    XXH3_64bits_withSeed(&intport, sizeof(intport), 0))));
        REQUIRE(hash != 0);
    }

    SECTION("XXH3: Check time for hashing") {
        u_int32_t extip = 4294967296 + 1;
        u_int32_t intip = 4294967296 + 1;
        u_int16_t extport = 65536 + 1;
        u_int16_t intport = 65536 + 1;
        double time1 = 0.0, tstart;
        tstart = clock(); // start
        XXH64_hash_t hash = XXH3_64bits_withSeed(
            &extip, sizeof(extip),
            XXH3_64bits_withSeed(
                &intip, sizeof(intip),
                XXH3_64bits_withSeed(
                    &extport, sizeof(extport),
                    XXH3_64bits_withSeed(&intport, sizeof(intport), 0))));
        time1 += clock() - tstart;      // end
        time1 = time1 / CLOCKS_PER_SEC; // rescale to seconds
        REQUIRE(time1 < 0.00001);
    }

    SECTION("calc_cookie_hash() returns 32 bit integer", "[]") {
        Treatment_friend treat;
        for (int i = 0; i < 1000; i++) {
            u_int32_t extip = (u_int32_t)rand();
            u_int32_t intip = (u_int32_t)rand();

            u_int16_t extport = (u_int16_t)rand();
            u_int16_t intport = (u_int16_t)rand();

            u_int32_t testValue =
                treat.calc_cookie_hash(0, extip, intip, extport, intport);

            CHECK(testValue != 0);
            CHECK(sizeof(testValue) == 4);
            //		BOOST_LOG_TRIVIAL(info)<<testValue;
        }
    }
    SECTION("calc_cookie_hash() returns same values if the input is the same",
            "[]") {
        Treatment_friend treat;

        u_int32_t extip1 = 12345;
        u_int32_t extip2 = 12345;
        u_int32_t intip1 = 98765;
        u_int32_t intip2 = 98765;

        u_int16_t extport1 = 123;
        u_int16_t extport2 = 123;
        u_int16_t intport1 = 987;
        u_int16_t intport2 = 987;

        u_int32_t testValue1 =
            treat.calc_cookie_hash(0, extip1, intip1, extport1, intport1);
        u_int32_t testValue2 =
            treat.calc_cookie_hash(0, extip2, intip2, extport2, intport2);

        REQUIRE(testValue1 == testValue2);
    }

    SECTION("calc_cookie_hash() returns same values if the input is the same "
            "but the objects are different",
            "[]") {
        // Create two different objects
        Treatment_friend treat1;
        Treatment_friend treat2;

        u_int32_t extip1 = 12345;
        u_int32_t intip1 = 98765;
        u_int16_t extport1 = 123;
        u_int16_t intport1 = 987;

        u_int32_t testValue1 =
            treat1.calc_cookie_hash(0, extip1, intip1, extport1, intport1);
        u_int32_t testValue2 =
            treat2.calc_cookie_hash(0, extip1, intip1, extport1, intport1);

        // Checks if the two hash values are the same
        REQUIRE(testValue1 == testValue2);
    }

    SECTION("calc_cookie_hash() has different values if _s_timestamp is not "
            "the same",
            "[]") {
        Treatment_friend treat;

        Data d;
        d._extip = 1234;
        d._intip = 9876;
        d._extport = 12;
        d._intport = 98;

        u_int32_t test1 =
            treat.calc_cookie_hash(treat.get_s_timestamp(), d._extip, d._intip,
                                   d._extport, d._intport);

        // increment _s_timestamp
        treat.s_increment_timestamp();
        // test if _s_timestamp was incremented
        CHECK(treat.get_s_timestamp() == 1);

        u_int32_t test2 =
            treat.calc_cookie_hash(treat.get_s_timestamp(), d._extip, d._intip,
                                   d._extport, d._intport);

        // increment _s_timestamp
        treat.s_increment_timestamp();
        // test if _s_timestamp was incremented
        CHECK(treat.get_s_timestamp() == 2);

        u_int32_t test3 =
            treat.calc_cookie_hash(treat.get_s_timestamp(), d._extip, d._intip,
                                   d._extport, d._intport);

        // Check if hash values (test1, test2, test3) are different
        CHECK(test1 != test2);
        CHECK(test2 != test3);
        CHECK(test1 != test3);
    }

    SECTION("calc_cookie_hash can handle smaller data types", "[]") {
        Treatment_friend treat;

        // Should be u_int32_t
        u_int16_t extip = 222;
        u_int16_t intip = 33;
        // Should be u_int16_t
        u_int8_t extport = 11;
        u_int8_t intport = 22;

        u_int32_t test = treat.calc_cookie_hash(treat.get_s_timestamp(), extip,
                                                intip, extport, intport);
        REQUIRE(test != 0);
    }

    SECTION("calc_cookie_hash can handle larger data types", "[]") {
        Treatment_friend treat;

        // Should be u_int32_t
        u_int64_t extip = 18446744073709551616 - 1; // == 2^64 - 1
        u_int64_t intip = 18446744073709551616;     // == 0
        // Should be u_int16_t
        u_int32_t extport = 4294967296 - 1; // == 2^32 - 1
        u_int32_t intport = 4294967296;     // == 0

        u_int32_t test = treat.calc_cookie_hash(treat.get_s_timestamp(), extip,
                                                intip, extport, intport);
        //	BOOST_LOG_TRIVIAL(info) << extip << " & " << intip;
        //	BOOST_LOG_TRIVIAL(info) << extport << " & " << intport;
        //	BOOST_LOG_TRIVIAL(info) << test;
        REQUIRE(test != 0);
    }
}

TEST_CASE("Map", "[]") {
    SECTION("Densemap: Insert elements", "[]") {
        google::dense_hash_map<Data, Info, TreatmentHash> densemap;

        // Dense_hash_map requires you call set_empty_key() immediately after
        // constructing the hash_map, and before calling any other
        // dense_hash_map method
        Data empty;
        empty._extip = 0;
        empty._intip = 0;
        empty._extport = 0;
        empty._intport = 0;
        densemap.set_empty_key(empty);

        Data d;
        d._extip = 12345;
        d._intip = 12334;
        d._extport = 123;
        d._intport = 1234;

        Info i(3, true, true, true, true, nullptr);
        densemap[d] = i;

        REQUIRE(densemap.empty() == false);
    }

    SECTION("Densemap:unchanged key after insertion", "[]") {
        google::dense_hash_map<Data, Info, TreatmentHash>
            densemap; // Data, Info and TreatmentHash defined in Treatment.h

        // Dense_hash_map requires to call set_empty_key() immediately after
        // constructing the hash_map, and before calling any other
        // dense_hash_map method
        Data empty;
        empty._extip = 0;
        empty._intip = 0;
        empty._extport = 0;
        empty._intport = 0;
        densemap.set_empty_key(empty);

        Data d;
        d._extip = 12345;
        d._intip = 12334;
        d._extport = 123;
        d._intport = 1234;

        Info i(3, true, true, true, true, nullptr);
        densemap[d] = i;
        auto id = densemap.find(d); // const_iterator find(const key_type& k)
                                    // const: Finds an element whose key is k

        CHECK(id->first._extip == d._extip);
        CHECK(id->first._intip == d._intip);
        CHECK(id->first._extport == d._extport);
        CHECK(id->first._intport == d._intport);
        CHECK(id->second._offset == 3);
        CHECK(id->second._finseen_to_inside == true);
        CHECK(id->second._finseen_to_outside == true);
        CHECK(id->second._ack_to_inside_expected == true);
        CHECK(id->second._ack_to_outside_expected == true);
    }

    SECTION("Densemap: Erase all elements", "[]") {
        google::dense_hash_map<Data, Info, TreatmentHash> densemap;

        // Dense_hash_map requires to call set_empty_key() immediately after
        // constructing the hash_map, and before calling any other
        // dense_hash_map method
        Data empty;
        empty._extip = 0;
        empty._intip = 0;
        empty._extport = 0;
        empty._intport = 0;
        densemap.set_empty_key(empty);

        Data d1;
        d1._extip = 12345;
        d1._intip = 12334;
        d1._extport = 123;
        d1._intport = 1234;

        Info i1(3, true, true, true, true, nullptr);
        densemap[d1] = i1;

        Data d2;
        d2._extip = 98765;
        d2._intip = 95678;
        d2._extport = 80;
        d2._intport = 81;

        Info i2(3, false, false, true, false, nullptr);
        densemap[d2] = i2;

        CHECK(densemap.size() == 2);

        densemap.clear(); // ereases all of the elements

        CHECK(densemap.empty() ==
              true); // bool empty() const: true if the dense_map's size is 0
    }

    SECTION("Densemap: Erase one element whose key is known", "[]") {
        google::dense_hash_map<Data, Info, TreatmentHash> densemap;

        // Dense_hash_map requires to call set_empty_key() immediately after
        // constructing the hash_map, and before calling any other
        // dense_hash_map method
        Data empty;
        empty._extip = 0;
        empty._intip = 0;
        empty._extport = 0;
        empty._intport = 0;
        densemap.set_empty_key(empty);

        // has to be defined at the very beginning of the programm to be able to
        // erase elements later
        Data deleted;
        deleted._extip = 0;
        deleted._intip = 0;
        deleted._extport = 1;
        deleted._intport = 1;
        densemap.set_deleted_key(deleted);

        Data d1;
        d1._extip = 12345;
        d1._intip = 12334;
        d1._extport = 123;
        d1._intport = 1234;

        Info i1(3, true, true, true, true, nullptr);
        densemap[d1] =
            i1; // calculates index over d, store d as first and i as second

        Data d2;
        d2._extip = 98765;
        d2._intip = 95678;
        d2._extport = 80;
        d2._intport = 81;

        Info i2(3, true, false, false, true, nullptr);
        densemap[d2] = i2;

        CHECK(densemap.size() == 2);
        densemap.erase(d1); // d is my key
        CHECK(densemap.size() == 1);
        densemap.erase(d2);
        CHECK(densemap.size() == 0);
    }

    /*
    This test fails like it should
    SECTION("Densemap: deleted_key is not different drom the key-value used for
    set_empty_key()"){ google::dense_hash_map<Data, Info, TreatmentHash>
    densemap;

            Data empty; // Dense_hash_map requires you call set_empty_kex()
    immediately after constructing the hash_map, and before calling any other
    dense_hash_map method empty._extip = 0; empty._intip = 0; empty._extport =
    0; empty._intport = 0; densemap.set_empty_key(empty);

            Data deleted; // Dense_hash_map requires you call set_empty_kex()
    immediately after constructing the hash_map, and before calling any other
    dense_hash_map method deleted._extip = 0; deleted._intip = 0;
            deleted._extport = 0;
            deleted._intport = 0;
            densemap.set_deleted_key(deleted); }	//has to be defined at
    the very beginning of the programm to be able to erase elements later }*/

    SECTION("Densemap: working with a lot of different data", "[]") {
        google::dense_hash_map<Data, Info, TreatmentHash> densemap;

        // Dense_hash_map requires to call set_empty_key() immediately after
        // constructing the hash_map, and before calling any other
        // dense_hash_map method
        Data empty;
        empty._extip = 0;
        empty._intip = 0;
        empty._extport = 0;
        empty._intport = 0;
        densemap.set_empty_key(empty);

        // has to be defined at the very beginning of the programm to be able to
        // erase elements later
        Data deleted;
        deleted._extip = 0;
        deleted._intip = 0;
        deleted._extport = 1;
        deleted._intport = 1;
        densemap.set_deleted_key(deleted);

        for (int j = 0; j < 1000000; j++) {
            Data d;
            d._extip = rand();
            d._intip = rand();
            d._extport = rand();
            d._intport = rand();

            Info i(3, false, true, false, true, nullptr);
            densemap[d] = i;
        }

        CHECK(densemap.size() == 1000000);

        Data d;
        d._extip = rand();
        d._intip = rand();
        d._extport = rand();
        d._intport = rand();

        Info i(3, false, false, true, true, nullptr);
        densemap[d] = i;

        CHECK(densemap.size() == 1000001);

        densemap.erase(d);

        CHECK(densemap.size() == 1000000);

        densemap.clear();

        CHECK(densemap.empty() == true);
    }

    SECTION("Densemap: working with the same data", "[]") {
        google::dense_hash_map<Data, Info, TreatmentHash> densemap;

        // Dense_hash_map requires to call set_empty_key() immediately after
        // constructing the hash_map, and before calling any other
        // dense_hash_map method
        Data empty;
        empty._extip = 0;
        empty._intip = 0;
        empty._extport = 0;
        empty._intport = 0;
        densemap.set_empty_key(empty);

        // has to be defined at the very beginning of the programm to be able to
        // erase elements later
        Data deleted;
        deleted._extip = 0;
        deleted._intip = 0;
        deleted._extport = 1;
        deleted._intport = 1;
        densemap.set_deleted_key(deleted);

        Data d;
        d._extip = 12345;
        d._intip = 54321;
        d._extport = 12;
        d._intport = 123;

        Info i1(3, true, true, true, true, nullptr);
        densemap[d] = i1;
        auto id1 = densemap.find(d);

        CHECK(densemap.size() == 1);
        CHECK(id1->first._extip == d._extip);
        CHECK(id1->first._intip == d._intip);
        CHECK(id1->first._extport == d._extport);
        CHECK(id1->first._intport == d._intport);
        CHECK(id1->second._offset == 3);
        CHECK(id1->second._finseen_to_inside == true);
        CHECK(id1->second._finseen_to_outside == true);
        CHECK(id1->second._ack_to_inside_expected == true);
        CHECK(id1->second._ack_to_outside_expected == true);

        Info i2(3, true, true, true, true, nullptr);
        densemap[d] = i2;

        CHECK(densemap.size() == 1);

        auto id2 = densemap.find(d);

        CHECK(id2->first._extip == d._extip);
        CHECK(id2->first._intip == d._intip);
        CHECK(id2->first._extport == d._extport);
        CHECK(id2->first._intport == d._intport);
        CHECK(id2->second._offset == 3);
        CHECK(id2->second._finseen_to_inside == true);
        CHECK(id2->second._finseen_to_outside == true);
        CHECK(id2->second._ack_to_inside_expected == true);
        CHECK(id2->second._ack_to_outside_expected == true);
    }

    SECTION("Densemap: Using insert instead of []", "[]") {
        google::dense_hash_map<Data, Info, TreatmentHash> _densemap;

        // Dense_hash_map requires to call set_empty_key() immediately after
        // constructing the hash_map, and before calling any other
        // dense_hash_map method
        Data empty;
        empty._extip = 0;
        empty._intip = 0;
        empty._extport = 0;
        empty._intport = 0;
        _densemap.set_empty_key(empty);

        // has to be defined at the very beginning of the programm to be able to
        // erase elements later
        Data deleted;
        deleted._intip = 0;
        deleted._extport = 1;
        deleted._intport = 1;
        _densemap.set_deleted_key(deleted);

        Data d(12345, 53210, 12, 1234);

        Info i(3, true, true, true, true, nullptr);
        _densemap.insert(std::pair<Data, Info>(d, i));

        CHECK(_densemap.empty() == false);
    }

    SECTION(
        "Densemap: Comparing if values after insertion are the same as before",
        "[]") {

        google::dense_hash_map<Data, Info, TreatmentHash> densemap;

        // Dense_hash_map requires to call set_empty_key() immediately after
        // constructing the hash_map, and before calling any other
        // dense_hash_map method
        Data empty;
        empty._extip = 0;
        empty._intip = 0;
        empty._extport = 0;
        empty._intport = 0;
        densemap.set_empty_key(empty);

        // has to be defined at the very beginning of the programm to be able to
        // erase elements later
        Data deleted;
        deleted._intip = 0;
        deleted._extport = 1;
        deleted._intport = 1;
        densemap.set_deleted_key(deleted);

        Data d;
        d._extip = 12345;
        d._intip = 54321;
        d._extport = 12;
        d._intport = 123;

        Info i(3, true, true, true, true, nullptr);
        densemap.insert(std::pair<Data, Info>(d, i));
        auto id = densemap.find(d);

        CHECK(id->first._extip == d._extip);
        CHECK(id->first._intip == d._intip);
        CHECK(id->first._extport == d._extport);
        CHECK(id->first._intport == d._intport);
        CHECK(id->second._offset == i._offset);
        CHECK(id->second._finseen_to_inside == i._finseen_to_inside);
    }
}

TEST_CASE("Creating a random number for cookie_secret()", "[]") {
    SECTION("Algorithm: Create random 64bit value", "[]") {

        for (int i = 0; i < 10000; i++) {
            u_int64_t random64BitNumber = 0;

            u_int64_t value1 = (uint16_t)rand();
            value1 = value1 << 48;
            random64BitNumber |= value1;

            u_int64_t value2 = (uint16_t)rand();
            value2 = value2 << 32;
            random64BitNumber |= value2;

            u_int64_t value3 = rand();
            random64BitNumber |= value3;

            //			BOOST_LOG_TRIVIAL(info) << random64BitNumber;
            CHECK(sizeof(random64BitNumber) == 8); // in byte: 8 Byte = 64 bit
        }
    }

    SECTION("Rand: check size", "[]") {

        Treatment_friend treat;
        for (int i = 0; i < 10000; i++) {
            u_int64_t r = Rand::get_random_64bit_value();
            //			BOOST_LOG_TRIVIAL(info) << r;
            CHECK(sizeof(r) == 8);
        }
    }

    SECTION("Rand: 1000 values are different", "[]") {

        Treatment_friend treat;
        int length = 1000;
        u_int64_t arr[length];

        for (int i = 0; i < length - 1; i++) {
            arr[i] = Rand::get_random_64bit_value();
        }

        for (int i = 0; i < length - 1; i++) {
            for (int j = 0; j < i; j++) {
                CHECK(arr[i] != arr[j]);
            }
        }
    }
}

TEST_CASE("check_syn_cookie()", "[]") {
    SECTION("Extract the last 8 bit of an u_int32_t") {
        uint32_t v1 = 4294967295;
        uint32_t v2 = v1 & 0xFF;
        CHECK(v2 == 0b11111111);

        uint32_t v3 = 22500;
        uint32_t v4 = v3 & 0xFF;
        CHECK(v4 == 0b11100100);

        uint32_t v5 = 2820803657;
        uint32_t v6 = v5 & 0xFF;
        CHECK(v6 == 0b01001001);
    }

    SECTION("check_syn_cookie(): diff == 0 (diff = _s_timestamp - "
            "cookie_timestamp = 0-0 = 0)(without using the PacketDissection)",
            "[]") {
        // Create a Treatment object
        Treatment_friend treat;

        u_int32_t extip = 12345;
        u_int32_t intip = 98765;
        u_int16_t extport = 123;
        u_int16_t intport = 124;

        // Create a cookie value
        u_int32_t cookie_value =
            treat.calc_cookie_hash(0, extip, intip, extport, intport);
        cookie_value = (cookie_value & 0xFFFFFF00);
        cookie_value |= (u_int8_t)0;

        // create an Data object
        Data d;
        d._extip = extip;
        d._intip = intip;
        d._extport = extport;
        d._intport = intport;

        REQUIRE(treat.check_syn_cookie(cookie_value, d));
    }

    SECTION("check_syn_cookie(): diff == 0, but false values in cookie_value "
            "(without using the PacketDissection)",
            "[]") {
        // Create a Treatment object
        Treatment_friend treat;

        u_int32_t extip = 12345;
        u_int32_t intip = 98765;
        u_int16_t extport = 123;
        u_int16_t intport = 124;

        // Create a cookie value
        u_int32_t cookie_value = treat.calc_cookie_hash(
            0, extip + 6, intip - 10, extport + 1, intport);
        cookie_value = (cookie_value & 0xFFFFFF00);
        cookie_value |= (u_int8_t)0;

        // create an Data object
        Data d;
        d._extip = extip;
        d._intip = intip;
        d._extport = extport;
        d._intport = intport;

        REQUIRE(treat.check_syn_cookie(cookie_value, d) == false);
    }

    SECTION("check_syn_cookie(): diff == 0 (diff = _s_timestamp - "
            "cookie_timestamp = 3-3 = 0)(without using the PacketDissection)",
            "[]") {
        // Create a Treatment object
        Treatment_friend treat;

        // increment _s_timestamp up to 3
        treat.s_increment_timestamp();
        treat.s_increment_timestamp();
        treat.s_increment_timestamp();
        CHECK(treat.get_s_timestamp() == 3);

        u_int32_t extip = 12345;
        u_int32_t intip = 98765;
        u_int16_t extport = 123;
        u_int16_t intport = 124;

        // Create a cookie value with timestamp 3
        u_int32_t cookie_value =
            treat.calc_cookie_hash(3, extip, intip, extport, intport);
        cookie_value = (cookie_value & 0xFFFFFF00);
        cookie_value |= (u_int8_t)3;

        // create a Data object
        Data d;
        d._extip = extip;
        d._intip = intip;
        d._extport = extport;
        d._intport = intport;

        CHECK(treat.check_syn_cookie(cookie_value, d));
    }

    SECTION("check_syn_cookie(): diff == 1 (diff = _s_timestamp - "
            "cookie_timestamp = 3- 2)(without using the PacketDissection)",
            "[]") {
        // Create a Treatment object
        Treatment_friend treat;

        // increment _s_timestamp up to 3
        treat.s_increment_timestamp();
        treat.s_increment_timestamp();
        treat.s_increment_timestamp();
        CHECK(treat.get_s_timestamp() == 3);

        u_int32_t extip = 12345;
        u_int32_t intip = 98765;
        u_int16_t extport = 123;
        u_int16_t intport = 124;

        // Create a cookie value with timestamp 2
        u_int32_t cookie_value =
            treat.calc_cookie_hash(2, extip, intip, extport, intport);
        cookie_value = (cookie_value & 0xFFFFFF00);
        cookie_value |= (u_int8_t)2;

        // create a Data object
        Data d;
        d._extip = extip;
        d._intip = intip;
        d._extport = extport;
        d._intport = intport;

        CHECK(treat.check_syn_cookie(cookie_value, d));
    }

    SECTION(
        "check_syn_cookie(): diff == 1 (without using the PacketDissection)",
        "[]") {
        // Create a Treatment object
        Treatment_friend treat;

        // increment _s_timestamp with _s_timestamp 1
        treat.s_increment_timestamp();
        CHECK(treat.get_s_timestamp() == 1);

        u_int32_t extip = 12345;
        u_int32_t intip = 98765;
        u_int16_t extport = 123;
        u_int16_t intport = 124;

        // Create a cookie value with timestamp 0
        u_int32_t cookie_value =
            treat.calc_cookie_hash(0, extip, intip, extport, intport);
        cookie_value = (cookie_value & 0xFFFFFF00);
        cookie_value |= (u_int8_t)0;

        // create a Data object
        Data d;
        d._extip = extip;
        d._intip = intip;
        d._extport = extport;
        d._intport = intport;

        CHECK(treat.check_syn_cookie(cookie_value, d));
    }

    SECTION("check_syn_cookie(): diff > 1 (diff = _s_timestamp - "
            "cookie_timestamp = 2- 0)(without using the PacketDissection)",
            "[]") {
        // Create a Treatment object
        Treatment_friend treat;

        // increment _s_timestamp up to 32
        treat.s_increment_timestamp();
        treat.s_increment_timestamp();
        CHECK(treat.get_s_timestamp() == 2);

        u_int32_t extip = 12345;
        u_int32_t intip = 98765;
        u_int16_t extport = 123;
        u_int16_t intport = 124;

        // Create a cookie value with timestamp 0
        u_int32_t cookie_value =
            treat.calc_cookie_hash(0, extip, intip, extport, intport);
        cookie_value = (cookie_value & 0xFFFFFF00);
        cookie_value |= (u_int8_t)0;

        // create a Data object
        Data d;
        d._extip = extip;
        d._intip = intip;
        d._extport = extport;
        d._intport = intport;

        CHECK(treat.check_syn_cookie(cookie_value, d) == false);
    }

    SECTION("check_syn_cookie(): diff == 1 with random numbers (without using "
            "the PacketDissection)",
            "[]") {
        // Create a Treatment object
        Treatment_friend treat;
        u_int8_t ran_num = rand();
        // increment _s_timestamp with _s_timestamp 6
        for (int i = 0; i < ran_num; i++) {
            treat.s_increment_timestamp();
        }

        CHECK(treat.get_s_timestamp() == ran_num);

        u_int32_t extip = rand();
        u_int32_t intip = rand();
        u_int16_t extport = rand();
        u_int16_t intport = rand();

        // Create a cookie value with timestamp 5
        u_int32_t cookie_value = treat.calc_cookie_hash(
            (ran_num - 1), extip, intip, extport, intport);
        cookie_value = cookie_value & 0xFFFFFF00;
        cookie_value |= (u_int8_t)(ran_num - 1);

        // create a Data object
        Data d;
        d._extip = extip;
        d._intip = intip;
        d._extport = extport;
        d._intport = intport;

        CHECK(treat.check_syn_cookie(cookie_value, d));
    }

    SECTION("check_syn_cookie(): diff > 1 (diff = _s_timestamp - "
            "cookie_timestamp = 64- 7)(without using the PacketDissection)",
            "[]") {
        // Create a Treatment object
        Treatment_friend treat;

        // increment _s_timestamp up to 64
        for (int i = 0; i < 64; i++) {
            treat.s_increment_timestamp();
        }
        CHECK(treat.get_s_timestamp() == 64);

        u_int32_t extip = 12345;
        u_int32_t intip = 98765;
        u_int16_t extport = 123;
        u_int16_t intport = 124;

        // Create a cookie value with timestamp 7
        u_int32_t cookie_value =
            treat.calc_cookie_hash(7, extip, intip, extport, intport);
        cookie_value = (cookie_value & 0xFFFFFF00);
        cookie_value |= (u_int8_t)7;

        // create a Data object
        Data d;
        d._extip = extip;
        d._intip = intip;
        d._extport = extport;
        d._intport = intport;

        CHECK(treat.check_syn_cookie(cookie_value, d) == false);
    }

    SECTION("check_syn_cookie(): diff < 0 (diff = _timestamp - "
            "cookie_timestamp = 0- 7)(without using the PacketDissection)",
            "[]") {
        // Create a Treatment object
        Treatment_friend treat;

        u_int32_t extip = 12345;
        u_int32_t intip = 98765;
        u_int16_t extport = 123;
        u_int16_t intport = 124;

        // Create a cookie value with timestamp 7
        u_int32_t cookie_value =
            treat.calc_cookie_hash(7, extip, intip, extport, intport);
        cookie_value = (cookie_value & 0xFFFFFF00);
        cookie_value |= (u_int8_t)7;

        // create a Data object
        Data d;
        d._extip = extip;
        d._intip = intip;
        d._extport = extport;
        d._intport = intport;

        CHECK(treat.check_syn_cookie(cookie_value, d) == false);
    }
}

TEST_CASE("Boost", "[]") {
    //	BOOST_LOG_TRIVIAL(info) << "Dies ist eine Info Message";
    //	BOOST_LOG_TRIVIAL(warning) << "Dies ist eine Warn-Nachricht";
}

TEST_CASE("UINTTEST", "[]") {
    u_int32_t yyy = 0x00000000;
    u_int32_t xxx = 0xFFFFFFFF;

    u_int32_t diff = yyy - xxx;
    REQUIRE(diff == 1);
}

TEST_CASE("Struct: Data", "[]") {

    SECTION("Default Constructor", "[]") {
        Data dat;

        // attributes have to be 0
        CHECK(dat._extip == 0);
        CHECK(dat._intip == 0);
        CHECK(dat._extport == 0);
        CHECK(dat._intport == 0);
    }

    SECTION("Data empty using member initializer lists", "[]") {
        Data empty(0, 0, 0, 0);

        CHECK(empty._extip == 0);
        CHECK(empty._intip == 0);
        CHECK(empty._extport == 0);
        CHECK(empty._intport == 0);
    }

    SECTION("Data empty without using member initializer lists", "[]") {
        Data empty;

        empty._extip = 0;
        empty._intip = 0;
        empty._extport = 0;
        empty._intport = 0;

        CHECK(empty._extip == 0);
        CHECK(empty._intip == 0);
        CHECK(empty._extport == 0);
        CHECK(empty._intport == 0);
    }

    SECTION("Data with random values using member initializer lists", "[]") {
        for (int i = 0; i < 1000; i++) {
            u_int32_t a = (u_int32_t)rand();
            u_int32_t b = (u_int32_t)rand();
            u_int16_t c = (u_int16_t)rand();
            u_int16_t d = (u_int16_t)rand();

            Data dat(a, b, c, d);

            CHECK(dat._extip == a);
            CHECK(dat._intip == b);
            CHECK(dat._extport == c);
            CHECK(dat._intport == d);
        }
    }

    SECTION("Data with random values without using member initializer lists",
            "[]") {
        Data dat;

        for (int i = 0; i < 1000; i++) {
            u_int32_t a = (u_int32_t)rand();
            u_int32_t b = (u_int32_t)rand();
            u_int16_t c = (u_int16_t)rand();
            u_int16_t d = (u_int16_t)rand();

            dat._extip = a;
            dat._intip = b;
            dat._extport = c;
            dat._intport = d;

            CHECK(dat._extip == a);
            CHECK(dat._intip == b);
            CHECK(dat._extport == c);
            CHECK(dat._intport == d);
        }
    }
}

TEST_CASE("Struct: Info", "[]") {
    SECTION("Default Constructor", "[]") {
        Info inf;

        CHECK(inf._offset == 0);
        CHECK(inf._finseen_to_inside == false);
        CHECK(inf._finseen_to_outside == false);
        CHECK(inf._ack_to_inside_expected == false);
        CHECK(inf._ack_to_outside_expected == false);
    }

    SECTION("Info with random values using member initializer lists", "[]") {
        for (int i = 0; i < 1000; i++) {
            int off = (int)rand();
            // random true or false
            bool fins1 = (rand() > (RAND_MAX / 2));
            bool fins2 = (rand() > (RAND_MAX / 2));
            bool fins3 = (rand() > (RAND_MAX / 2));
            bool fins4 = (rand() > (RAND_MAX / 2));

            Info inf(off, fins1, fins2, fins3, fins4, nullptr);

            CHECK(inf._offset == off);
            CHECK(inf._finseen_to_inside == fins1);
            CHECK(inf._finseen_to_outside == fins2);
            CHECK(inf._ack_to_inside_expected == fins3);
            CHECK(inf._ack_to_outside_expected == fins4);
        }
    }

    SECTION("Info with random values without using member initializer lists",
            "[]") {
        for (int i = 0; i < 1000; i++) {
            int off = (int)rand();
            // random true or false
            bool fins1 = (rand() > (RAND_MAX / 2));
            bool fins2 = (rand() > (RAND_MAX / 2));
            bool fins3 = (rand() > (RAND_MAX / 2));
            bool fins4 = (rand() > (RAND_MAX / 2));

            Info inf;

            inf._offset = off;
            inf._finseen_to_inside = fins1;
            inf._finseen_to_outside = fins2;
            inf._ack_to_inside_expected = fins3;
            inf._ack_to_outside_expected = fins4;
            CHECK(inf._offset == off);
            CHECK(inf._finseen_to_inside == fins1);
            CHECK(inf._finseen_to_outside == fins2);
            CHECK(inf._ack_to_inside_expected == fins3);
            CHECK(inf._ack_to_outside_expected == fins4);
        }
    }
}

TEST_CASE("s_increment_timestamp", "[]") {
    SECTION("Increment _timestamp up to 255 (size of u_int8_t)", "[]") {
        Treatment_friend treat;
        for (u_int8_t i = 0; i < 255; i++) { // 255 = 2^8 -1
            CHECK(treat.get_s_timestamp() == i);
            treat.s_increment_timestamp();
        }
        CHECK(treat.get_s_timestamp() == 255);
    }
    SECTION("Increment _s_timestamp up to 1000 (>255>size if u_int8_t)", "[]") {
        Treatment_friend treat;

        u_int8_t count = 0;

        for (int i = 0; i < 1000; i++) {
            CHECK(treat.get_s_timestamp() == count);
            treat.s_increment_timestamp();
            count++; // everytimes when count would reache 256, it will start
                     // with 0 since the size of u_int8_t is 255
        }
    }

    SECTION("Change the value of _s_timestamp manually, increment _s_timestamp "
            "should still work after this change") {
        Treatment_friend treat;

        CHECK(treat.get_s_timestamp() == 0);

        // Change the value of _s_timestamp to 45 (higher value than before)
        treat.set_s_timestamp(45);
        CHECK(treat.get_s_timestamp() == 45);

        treat.s_increment_timestamp();
        CHECK(treat.get_s_timestamp() == 46);

        // Change the value of _s_timestamp to 45 (lower value than before)
        treat.set_s_timestamp(22);
        CHECK(treat.get_s_timestamp() == 22);

        treat.s_increment_timestamp();
        CHECK(treat.get_s_timestamp() == 23);
    }
}
TEST_CASE("Benchmark", "[]") {
    typedef std::unordered_map<Data, Info, TreatmentHash> unordered;
    unordered unord;
    google::dense_hash_map<Data, Info, TreatmentHash> densemap;
    clock_t tu;
    // clock_t tr;
    clock_t td;
    Data empty;
    empty._extip = 0;
    empty._extport = 0;
    empty._intip = 0;
    empty._intport = 0;
    densemap.set_empty_key(empty);
    Info flix(0, true, true, true, true, nullptr);

    // -------------------------------------------------------------------------------------------------------

    //-----------------------------------------------------
    long runs = 1;
    clock_t uclock[runs] = {};
    clock_t dclock[runs] = {};
    long runner = 600000;
    Data arr[runner] = {};

    for (long r = 0; r < runs; ++r) {

        for (long i = 0; i < runner; ++i) {
            arr[i]._extip = rand();
            arr[i]._intip = rand();
            arr[i]._extport = rand();
            arr[i]._intport = rand();
        }

        auto startu = std::chrono::high_resolution_clock::now();
        tu = clock();
        for (long i = 0; i < runner; ++i) {
            unord.emplace(arr[i], flix);
        }

        for (long i = 0; i < runner; ++i) {
            unord.find(arr[i - 1 % runner]);
            unord.find(arr[i]);
            unord.find(arr[i + 1 % runner]);
            unord.find(arr[i + 50 % runner]);
        }
        tu = clock() - tu;
        auto finishu = std::chrono::high_resolution_clock::now();

        auto startd = std::chrono::high_resolution_clock::now();
        td = clock();
        for (long i = 0; i < runner; ++i) {
            densemap.insert(std::pair<Data, Info>(
                arr[i], flix)); // insert rather than densemap[arr[i]]
        }
        for (long i = 0; i < runner; ++i) {
            densemap.find(arr[i - 1 % runner]);
            densemap.find(arr[i]);
            densemap.find(arr[i + 1 % runner]);
            densemap.find(arr[i + 50 % runner]);
        }
        td = clock() - td;
        auto finishd = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsedu = finishu - startu;
        std::chrono::duration<double> elapsedd = finishd - startd;
        dclock[r] = td;
        uclock[r] = tu;
        BOOST_LOG_TRIVIAL(info)
            << "Elapsed time of unordered: " << elapsedu.count();
        BOOST_LOG_TRIVIAL(info)
            << "Elapsed time of dense: " << elapsedd.count();
    }
    int sumd = 0;
    int sumu = 0;
    for (long x = 0; x < runs; ++x) {
        sumd = sumd + dclock[x];
        sumu = sumu + uclock[x];
    }
    BOOST_LOG_TRIVIAL(info)
        << "This is the average clock count of densemap of " << runs
        << " rounds, of each " << runner << " elements inserted, and "
        << 4 * runner << " elements searched : " << sumd / runs;
    BOOST_LOG_TRIVIAL(info)
        << "This is the average clock count of unordered_map of " << runs
        << " rounds, of each " << runner << " elements inserted, and "
        << 4 * runner << " elements searched : " << sumu / runs;
}
