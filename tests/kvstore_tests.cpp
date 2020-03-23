#include <string>

#include "catch.hpp"

#include "data/dataframe.h"
#include "store/kvstore.h"

SCENARIO("Can construct a KVStore containing a Dataframe constructed from double array"){
    GIVEN("An array of doubles and a KVStore (with key)"){
        size_t SZ = 1000*1000;
        double* vals = new double[SZ];
        double sum = 0;
        for (size_t i = 0; i < SZ; ++i) sum += vals[i] = i;
        KVStore kvs;
        KVStore::Key k(std::string("triv"), 0);
        WHEN("A Dataframe is constructed using from array") {
            auto df = DataFrame::from_array(kvs, k, vals, SZ);
            REQUIRE(df->get_double(0, 1) == 1);
            auto df2 = kvs.get(k);
            for(size_t i = 0; i < SZ; ++i) sum -= *df2->get_double(0, i);
            THEN("The constructed dataframe is in the KVStore"){
                REQUIRE(sum == 0);
            }
        }
        delete[] vals;
    }
}

SCENARIO("Can construct a KVStore containing a Dataframe constructed from integer array"){
    GIVEN("An array of integers and a KVStore (with key)"){
        size_t SZ = 1000*1000;
        int* vals = new int[SZ];
        int sum = 0;
        for (size_t i = 0; i < SZ; ++i) sum += vals[i] = i;
        KVStore kvs;
        KVStore::Key k(std::string("triv"), 0);
        WHEN("A Dataframe is constructed using from array") {
            auto df = DataFrame::from_array(kvs, k, vals, SZ);
            REQUIRE(df->get_int(0, 1) == 1);
            auto df2 = kvs.get(k);
            for(size_t i = 0; i < SZ; ++i) sum -= *df2->get_int(0, i);
            THEN("The constructed dataframe is in the KVStore"){
                REQUIRE(sum == 0);
            }
        }
        delete[] vals;
    }
}

SCENARIO("Can construct a KVStore containing a Dataframe constructed from boolean array"){
    GIVEN("An array of booleans and a KVStore (with key)"){
        size_t SZ = 1000*1000;
        bool* vals = new bool[SZ];
        int sum = 0;
        for (size_t i = 0; i < SZ; ++i) sum += vals[i] = i;
        KVStore kvs;
        KVStore::Key k(std::string("triv"), 0);
        WHEN("A Dataframe is constructed using from array") {
            auto df = DataFrame::from_array(kvs, k, vals, SZ);
            REQUIRE(df->get_bool(0, 1) == true);
            auto df2 = kvs.get(k);
            for(size_t i = 0; i < SZ; ++i) sum -= *df2->get_bool(0, i);
            THEN("The constructed dataframe is in the KVStore"){
                REQUIRE(sum == 0);
            }
        }
        delete[] vals;
    }
}
