#include "RandomNumberGenerator.hpp"
#include <catch2/catch.hpp>
#include <iostream>
#include <time.h> // is used for testing the time in the 3rd test case

TEST_CASE("random_number_generator_basic", "[]") {

    // This test was written to check basic functions like whether different
    // numbers are generated.
    SECTION("check_whether_different_16", "[]") {
        // creates a new RNG object
        RandomNumberGenerator* xor_shift = new RandomNumberGenerator();

        // creates two pseudo random 16 bit numbers
        u_int16_t test_1 = xor_shift->gen_rdm_16_bit();
        u_int16_t test_2 = xor_shift->gen_rdm_16_bit();

        // printes these numbers out
        std::cout << "1st generated 16 bit int: " << test_1 << std::endl;
        std::cout << "2nd generated 16 bit int: " << test_2 << std::endl;

        // checks whether these numbers are different
        // test1 == test2 wouldn't means that the test isn't random.
        // This section just exists to see whether the algorithm basically works
        // and generates numbers.
        CHECK(test_1 != test_2);
    }

    // The same test like above but for the 32 bit algorithm:
    SECTION("CheckWhetherDifferent32", "[]") {
        RandomNumberGenerator xor_shift;
        u_int32_t test_1 = xor_shift.gen_rdm_32_bit();
        u_int32_t test_2 = xor_shift.gen_rdm_32_bit();
        std::cout << "1st generated 32 bit int: " << test_1 << std::endl;
        std::cout << "2nd generated 32 bit int: " << test_2 << std::endl;
        CHECK(test_1 != test_2);
    }

    // The same test like above but for the 64 bit algorithm:
    SECTION("CheckWhetherDifferent64", "[]") {
        RandomNumberGenerator xor_shift;
        u_int64_t test_1 = xor_shift.gen_rdm_64_bit();
        u_int64_t test_2 = xor_shift.gen_rdm_64_bit();
        std::cout << "1st generated 64 bit int: " << test_1 << std::endl;
        std::cout << "2nd generated 64 bit int: " << test_2 << std::endl;
        CHECK(test_1 != test_2);
        // empty line for better layout in testlog
        std::cout << std::endl;
    }

    // This test checks the type of the return value.
    // This particular section is for 16 bit
    SECTION("CheckSize16", "[]") {
        // Creates a RNG and generates a number like above
        RandomNumberGenerator xor_shift;
        u_int16_t test_value = xor_shift.gen_rdm_16_bit();
        std::cout << "values that are generated for checking the size: "
                  << std::endl;
        std::cout << "generated value: " << test_value << std::endl;
        // checks wheter the size of the return value is 16 bit or 2 byte
        CHECK(sizeof(test_value) == 2);
    }

    // The same for 32 bit
    SECTION("CheckSize32", "[]") {
        RandomNumberGenerator xor_shift;
        u_int32_t test_value = xor_shift.gen_rdm_32_bit();
        std::cout << "generated value (32 bit): " << test_value << std::endl;
        // checks wheter the size of the return value is 32 bit or 4 byte
        CHECK(sizeof(test_value) == 4);
    }

    // The same for 64 bit
    SECTION("CheckSize64", "[]") {
        RandomNumberGenerator xor_shift;
        u_int64_t test_value = xor_shift.gen_rdm_64_bit();
        std::cout << "generated value (64 bit): " << test_value << std::endl;
        // checks wheter the size of the return value is 64 bit or 8 byte
        CHECK(sizeof(test_value) == 8);
        std::cout << std::endl;
    }

    // Checks whether different numbers are generated when using different RNGs
    SECTION(
        "Check whether different when using different RNG objects for 16 bit",
        "[]") {
        // creating two objects
        RandomNumberGenerator xor_shift_1;
        RandomNumberGenerator xor_shift_2;
        // generating two values of 16 bit
        std::cout << "16 bit seed 1: " << xor_shift_1._seed_x16 << std::endl;
        std::cout << "16 bit seed 2: " << xor_shift_2._seed_x16 << std::endl;
        u_int16_t test_1_16_bit = xor_shift_1.gen_rdm_16_bit();
        u_int16_t test_2_16_bit = xor_shift_2.gen_rdm_16_bit();
        CHECK(test_1_16_bit != test_2_16_bit);
    }

    // the same for 32 bit again:
    SECTION(
        "Check whether different when using different RNG objects for 32 bit",
        "[]") {
        RandomNumberGenerator xor_shift_1;
        RandomNumberGenerator xor_shift_2;
        std::cout << "32 bit seed 1: " << xor_shift_1._seed_x32 << std::endl;
        std::cout << "32 bit seed 2: " << xor_shift_2._seed_x32 << std::endl;
        u_int32_t test_1_32_bit = xor_shift_1.gen_rdm_32_bit();
        u_int32_t test_2_32_bit = xor_shift_2.gen_rdm_32_bit();
        CHECK(test_1_32_bit != test_2_32_bit);
    }

    // the same for 64 bit again:
    SECTION(
        "Check whether different when using different RNG objects for 64 bit",
        "[]") {
        RandomNumberGenerator xor_shift_1;
        RandomNumberGenerator xor_shift_2;
        std::cout << "64 bit seed 1: " << xor_shift_1._seed_x64 << std::endl;
        std::cout << "64 bit seed 2: " << xor_shift_2._seed_x64 << std::endl;
        u_int64_t test_1_64_bit = xor_shift_1.gen_rdm_64_bit();
        u_int64_t test_2_64_bit = xor_shift_2.gen_rdm_64_bit();
        CHECK(test_1_64_bit != test_2_64_bit);
        std::cout << std::endl;
    }

    // This test checks whether two RNGs generate the same number after the seed
    // is set to the same number
    SECTION("Check whether the same numbers are generated with the same seed "
            "for 16 bit",
            "[]") {
        RandomNumberGenerator xor_shift_1;
        RandomNumberGenerator xor_shift_2;
        // set the seed to the same value in both RNGs
        xor_shift_1._seed_x16 = 30000;
        xor_shift_2._seed_x16 = 30000;
        std::cout << "16 bit seed for RNG 1: " << xor_shift_1._seed_x16
                  << std::endl;
        std::cout << "16 bit seed for RNG 2: " << xor_shift_2._seed_x16
                  << std::endl;
        u_int16_t test_1_16_bit = xor_shift_1.gen_rdm_16_bit();
        u_int16_t test_2_16_bit = xor_shift_2.gen_rdm_16_bit();
        std::cout << "number generated from RNG 1: " << xor_shift_1._seed_x16
                  << std::endl;
        std::cout << "number generated from RNG 2: " << xor_shift_2._seed_x16
                  << std::endl;
        // check whether the results are the same too
        CHECK(test_1_16_bit == test_2_16_bit);
        std::cout << std::endl;
    }

    // the same test for 32 bit
    SECTION("Check whether the same numbers are generated with the same seed "
            "for 32 bit",
            "[]") {
        RandomNumberGenerator xor_shift_1;
        RandomNumberGenerator xor_shift_2;
        // set the seed to the same value in both RNGs
        xor_shift_1._seed_x32 = 30000000;
        xor_shift_2._seed_x32 = 30000000;
        std::cout << "32 bit seed for RNG 1: " << xor_shift_1._seed_x32
                  << std::endl;
        std::cout << "32 bit seed for RNG 2: " << xor_shift_2._seed_x32
                  << std::endl;
        u_int32_t test_1_32_bit = xor_shift_1.gen_rdm_32_bit();
        u_int32_t test_2_32_bit = xor_shift_2.gen_rdm_32_bit();
        std::cout << "number generated from RNG 1: " << xor_shift_1._seed_x32
                  << std::endl;
        std::cout << "number generated from RNG 2: " << xor_shift_2._seed_x32
                  << std::endl;
        // check whether the results are the same too
        CHECK(test_1_32_bit == test_2_32_bit);
        std::cout << std::endl;
    }

    // the same test for 64 bit
    SECTION("Check whether the same numbers are generated with the same seed "
            "for 64 bit",
            "[]") {
        RandomNumberGenerator xor_shift_1;
        RandomNumberGenerator xor_shift_2;
        // set the seed to the same value in both RNGs
        xor_shift_1._seed_x64 = 30000000000;
        xor_shift_2._seed_x64 = 30000000000;
        std::cout << "64 bit seed for RNG 1: " << xor_shift_1._seed_x64
                  << std::endl;
        std::cout << "64 bit seed for RNG 2: " << xor_shift_2._seed_x64
                  << std::endl;
        u_int64_t test_1_64_bit = xor_shift_1.gen_rdm_64_bit();
        u_int64_t test_2_64_bit = xor_shift_2.gen_rdm_64_bit();
        std::cout << "number generated from RNG 1: " << xor_shift_1._seed_x64
                  << std::endl;
        std::cout << "number generated from RNG 2: " << xor_shift_2._seed_x64
                  << std::endl;
        // check whether the results are the same too
        CHECK(test_1_64_bit == test_2_64_bit);
        std::cout << std::endl;
    }

    SECTION("Check whether generated numbers are really in the interval",
            "[]") {
        RandomNumberGenerator xor_shift;
        u_int16_t test_value;
        int lower_limit = 1024;
        int upper_limit = 49151;
        bool no_number_has_been_outside_the_interval = true;
        // inside the for loop an if statement checks for 1,000,000 generated
        // numbers whether they are really in the interval
        for (int i = 0; i < 10000000; i++) {
            test_value =
                xor_shift.gen_rdm_16_bit_in_interval(lower_limit, upper_limit);
            if (test_value < lower_limit || test_value > upper_limit) {
                no_number_has_been_outside_the_interval = false;
            }
        }
        std::cout << "No generated number has been outside the interval? (1 "
                     "means true) --> "
                  << no_number_has_been_outside_the_interval << std::endl;
        std::cout << std::endl;
        CHECK(no_number_has_been_outside_the_interval == true);
    }
}

TEST_CASE("RandomNumberGeneratorStatistics", "[]") {

    // The result of the chi square test shows how uniform the generated numbers
    // are distributed
    // A big chi square means that the actual frequencies vary widely from the
    // theoretical frequencies.
    SECTION("ChiSquare16", "[]") {
        RandomNumberGenerator xor_shift;
        // 65536 - 1 = 2 ^ 16 different numbers can be generated
        int r = 65536 - 1;
        // 1,000,000 numbers are generated
        int n = 1000000;
        u_int16_t t;
        // this array counts how often each number from 0 to r is returned as a
        // result
        int f[r] = {};
        for (int i = 0; i < r; i++) {
            f[i] = 0;
        }
        for (int i = 1; i < n; i++) {
            t = xor_shift.gen_rdm_16_bit_in_interval(1024, 49151);
            f[t]++;
        }
        double chisquare = 0.0;
        for (int i = 0; i < r; i++) {
            // chi square is calculated
            chisquare = chisquare + ((f[i] - n / r) * (f[i] - n / r) / (n / r));
        }
        std::cout << "+++ chi square test for gen_rdm_16_bit_in_interval() +++"
                  << std::endl;
        std::cout << "chi square is: " << chisquare << std::endl;
        double k = sqrt(chisquare / (n + chisquare));
        std::cout << "k is: " << k << std::endl;

        // k is in [0; k_max] with k_max ≈ 1
        // Calculating k_norm wouldn't make sense.
        // 0 means that every number is generated equally frequent
        // 1 means not random at all

        // A bad result could be improved by returning _seed_x16 instead of
        // _seed_x16 % 48128 + 1024 (valid port number) in
        // RandomNumberGenerator.cpp.

        CHECK(k < 1.0);

        std::cout << std::endl;
    }

    SECTION("ChiSquare16", "[]") {
        RandomNumberGenerator xor_shift;
        // 65526 - 1 = 2 ^ 16 different numbers can be generated
        int r = 65536 - 1;
        // 1,000,000 numbers are generated
        int n = 1000000;
        u_int16_t t;
        // this array counts how often each number from 0 to r is returned as a
        // result
        int f[r] = {};
        for (int i = 0; i < r; i++) {
            f[i] = 0;
        }
        for (int i = 1; i < n; i++) {
            t = xor_shift.gen_rdm_16_bit();
            f[t]++;
        }
        double chisquare = 0.0;
        for (int i = 0; i < r; i++) {
            // chi square is calculated
            chisquare = chisquare + ((f[i] - n / r) * (f[i] - n / r) / (n / r));
        }
        std::cout << "+++ chi square test for gen_rdm_16_bit() +++"
                  << std::endl;
        std::cout << "chi square is: " << chisquare << std::endl;
        double k = sqrt(chisquare / (n + chisquare));
        std::cout << "k is: " << k << std::endl;

        CHECK(k < 1.0);

        std::cout << std::endl;
    }

    // the following test fails due to an segmentation violation signal
    // 32 bit seems to be to big

    /*SECTION("ChiSquare32", "[]") {
        RandomNumberGenerator xor_shift;
        u_int32_t r = 4294967296 - 1;
        u_int64_t n = 10000000000;
        u_int32_t t;
        int f[r] = {};
        for (u_int64_t i = 0; i < r; i++) {
            f[i] = 0;
        }
        for (u_int64_t i = 1; i < n; i++) {
            t = xor_shift.gen_rdm_32_bit();
            f[t]++;
        }
        double chisquare = 0.0;
        for (int i = 0; i < r; i++) {
            chisquare = chisquare + ((f[i] - n / r) * (f[i] - n / r) / (n / r));
        }
        std::cout << "chi square is: " << chisquare << std::endl;
        double k = sqrt(chisquare / (n + chisquare));
        std::cout << "k is: " << k << std::endl;
        // k is in [0; k_max] with k_max ≈ 1
        // 0 means independence
        // 1 means not random at all
        CHECK(k < 1.0);
    }*/
}

TEST_CASE("RandomNumberGeneratorTime", "[]") {

    // The following section is calculating the time for generating n
    // 16 bit numbers with a single RNG object
    SECTION("TestTime16", "[]") {
        // the following two lines initialize and start the timer
        double time1 = 0.0, tstart;
        tstart = clock();
        // creating a RNG object
        RandomNumberGenerator xor_shift;
        // amount of numbers generated
        // can be changed if neccessary, but it you will get a segmentation
        // violation error if it's to big
        long n = 10000000;
        // variable to store generated number
        uint16_t test_value;
        // generating those numbers
        for (long i = 0; i < n; i++) {
            test_value = xor_shift.gen_rdm_16_bit();
        }
        // stops the timer and calculates the difference between start and end
        time1 += clock() - tstart;
        // prints out the time
        std::cout << "time needed to generate " << n
                  << " 16 bit numbers: " << time1 / CLOCKS_PER_SEC << " s"
                  << std::endl;
        CHECK(time1 / CLOCKS_PER_SEC < 10.0);
    }

    // This test calculates the time needed to generate a certain amount of 16
    // bit ints with rand() allowowing a comparison with xorShift
    SECTION("TestTime16Rand", "[]") {
        double time1 = 0.0, tstart;
        tstart = clock();
        long n = 10000000;
        uint16_t test_value;
        for (long i = 0; i < n; i++) {
            test_value = rand();
        }
        time1 += clock() - tstart;
        std::cout << "time needed to generate " << n
                  << " 16 bit numbers with rand(): " << time1 / CLOCKS_PER_SEC
                  << " s" << std::endl;
        CHECK(time1 / CLOCKS_PER_SEC < 10.0);
    }

    // the same test for 32 bit numbers
    SECTION("TestTime32", "[]") {
        double time1 = 0.0, tstart;
        tstart = clock();
        RandomNumberGenerator xor_shift;
        long n = 10000000;
        uint32_t test_value;
        for (long i = 0; i < n; i++) {
            test_value = xor_shift.gen_rdm_32_bit();
        }
        time1 += clock() - tstart;
        std::cout << "time needed to generate " << n
                  << " 32 bit numbers: " << time1 / CLOCKS_PER_SEC << " s"
                  << std::endl;
        CHECK(time1 / CLOCKS_PER_SEC < 1.0);
    }

    // true 32 bit numbers with shifting and rand() for comparison to the
    // section above
    SECTION("TestTime32Rand", "[]") {
        double time1 = 0.0, tstart;
        tstart = clock();
        long n = 10000000;
        uint32_t test_value;
        for (long i = 0; i < n; i++) {
            test_value = (uint16_t)rand();
            test_value |= (uint16_t)rand() << 16;
        }
        time1 += clock() - tstart;
        std::cout << "time needed to generate " << n
                  << " 32 bit numbers with rand() and shifting: "
                  << time1 / CLOCKS_PER_SEC << " s" << std::endl;
        CHECK(time1 / CLOCKS_PER_SEC < 1.0);
    }

    // the same test for 64 bit numbers
    SECTION("TestTime64", "[]") {
        double time1 = 0.0, tstart;
        tstart = clock();
        RandomNumberGenerator xor_shift;
        long n = 10000000;
        uint64_t test_value;
        for (long i = 0; i < n; i++) {
            test_value = xor_shift.gen_rdm_64_bit();
        }
        time1 += clock() - tstart;
        std::cout << "time needed to generate " << n
                  << " 64 bit numbers: " << time1 / CLOCKS_PER_SEC << " s"
                  << std::endl;
        CHECK(time1 / CLOCKS_PER_SEC < 1.0);
    }

    // true 64 bit numbers with shifting and rand() for comparison to the
    // section above
    SECTION("TestTime64Rand", "[]") {
        double time1 = 0.0, tstart;
        tstart = clock();
        long n = 10000000;
        uint64_t test_value = 0;
        for (long i = 0; i < n; i++) {
            // the following lines have been copied from
            // Treatment::create_cookie_secret()
            u_int64_t value1 = (uint16_t)rand();
            value1 = (value1 << 48);
            test_value |= value1;
            u_int64_t value2 = (uint16_t)rand();
            value2 = (value2 << 32);
            test_value |= value2;
            u_int64_t value3 = (uint16_t)rand();
            test_value |= value3;
        }
        time1 += clock() - tstart;
        std::cout << "time needed to generate " << n
                  << " 64 bit numbers with rand() and shifting: "
                  << time1 / CLOCKS_PER_SEC << " s" << std::endl;
        CHECK(time1 / CLOCKS_PER_SEC < 1.0);
    }
}