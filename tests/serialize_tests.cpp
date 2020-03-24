#include <memory>
#include <iostream>

#include "catch.hpp"
#include "test_util.h"
#include "test_rower.h"

#include "util/serializable.h"
#include "data/dataframe.h"
#include "data/schema.h"
#include "data/column.h"

// this helper has the benefit of checking move constructors properly
// work by switching scopes and forcing a delete of the stack allocated object
// created by deserialize
std::unique_ptr<Schema> deserialize_schema_and_alloc(std::vector<uint8_t> data, size_t& pos) {
    return std::make_unique<Schema>(Serializable::deserialize<Schema>(data, pos));
}

std::shared_ptr<DataFrame> deserialize_dataframe_and_alloc(std::vector<uint8_t> data, size_t& pos) {
    return std::make_shared<DataFrame>(Serializable::deserialize<DataFrame>(data, pos));
}

std::unique_ptr<IntColumn> deserialize_intcolumn_and_alloc(std::vector<uint8_t> data, size_t& pos) {
    return std::make_unique<IntColumn>(Serializable::deserialize<IntColumn>(data, pos));
}

std::unique_ptr<FloatColumn> deserialize_floatcolumn_and_alloc(std::vector<uint8_t> data, size_t& pos) {
    return std::make_unique<FloatColumn>(Serializable::deserialize<FloatColumn>(data, pos));
}

std::unique_ptr<BoolColumn> deserialize_boolcolumn_and_alloc(std::vector<uint8_t> data, size_t& pos) {
    return std::make_unique<BoolColumn>(Serializable::deserialize<BoolColumn>(data, pos));
}

std::unique_ptr<StringColumn> deserialize_strcolumn_and_alloc(std::vector<uint8_t> data, size_t& pos) {
    return std::make_unique<StringColumn>(Serializable::deserialize<StringColumn>(data, pos));
}

bool compare_serialized(std::vector<uint8_t> one, std::vector<uint8_t> two) {
    if(one.size() == two.size()) {
        for(size_t i = 0; i < one.size(); ++i) {
            if(one[i] != two[i]) return false;
        }
        return true;
    }
    return false;
}

void push_to_vec(std::vector<uint8_t>& vec, uint8_t *buf, size_t len, const void *value) {
    memcpy(buf, value, len);
    for(size_t i = 0; i < len; ++i){
        vec.push_back(buf[i]);
    }
}


SCENARIO("We can properly serialize and deserialize objects") {

    GIVEN("A basic int column.") {
        IntColumn ic(5, 1, 2, 3, 4, 5);
        WHEN("We serialize it.") {
            std::vector<uint8_t> serialized = ic.serialize();
            THEN("It is correctly serialized"){
                std::vector<uint8_t> expected;
                uint8_t local_buf[sizeof(size_t)];
                size_t len = 5;
                push_to_vec(expected, local_buf, sizeof(size_t), &len);
                bool exists = true;
                for(int i = 1; i < 6; ++i) {
                    push_to_vec(expected, local_buf, sizeof(bool), &exists);
                    push_to_vec(expected, local_buf, sizeof(int), &i);
                }

                REQUIRE(compare_serialized(serialized, expected));
            }

            THEN("We can deserialize it.") {
                size_t pos = 0;
                IntColumn d_ic = Serializable::deserialize<IntColumn>(serialized, pos);
                REQUIRE(ic.size() == d_ic.size());
                REQUIRE(ic.get(0) == d_ic.get(0));
                REQUIRE(ic.get(1) == d_ic.get(1));
                REQUIRE(ic.get(2) == d_ic.get(2));
                REQUIRE(ic.get(3) == d_ic.get(3));
                REQUIRE(ic.get(4) == d_ic.get(4));
                REQUIRE(d_ic.equals(&ic));
            }
        }
    }

    GIVEN("A float column with a missing value") {
        FloatColumn fc(3, 1.1, 2.2, 3.3);
        fc.push_back(std::nullopt);
        WHEN("We serialize it.") {
            std::vector<uint8_t> serialized = fc.serialize();

            THEN("It is properly Serialized") {
                std::vector<uint8_t> expected;
                uint8_t local_buf[sizeof(size_t)];
                size_t len = 4;
                push_to_vec(expected, local_buf, sizeof(size_t), &len);
                bool exists = true;
                double values[] = {1.1, 2.2, 3.3};
                for(size_t i = 0; i < 3; ++i) {
                    push_to_vec(expected, local_buf, sizeof(bool), &exists);
                    push_to_vec(expected, local_buf, sizeof(double), values + i);
                }
                exists = false;
                push_to_vec(expected, local_buf, sizeof(bool), &exists);

                REQUIRE(compare_serialized(serialized, expected));
            }
            THEN("We can deserialize it."){
                size_t pos = 0;
                FloatColumn d_fc = Serializable::deserialize<FloatColumn>(serialized, pos);
                REQUIRE(fc.size() == d_fc.size());
                REQUIRE((*fc.get(0) - *d_fc.get(0)) < 0.001);
                REQUIRE((*fc.get(1) - *d_fc.get(1)) < 0.001);
                REQUIRE((*fc.get(2) - *d_fc.get(2)) < 0.001);
                REQUIRE((*fc.get(3) - *d_fc.get(3)) < 0.001);
                REQUIRE((!fc.get(3) && !d_fc.get(3)));
                REQUIRE(d_fc.equals(&fc));
            }
        }
    }

    GIVEN("A String Column with missing values") {
        StringColumn sc(1, "test");
        sc.push_back(std::nullopt);

        WHEN("We serialize it.") {
            std::vector<uint8_t> serialized = sc.serialize();

            THEN("It is properly Serialized") {
                std::vector<uint8_t> expected;
                uint8_t local_buf[sizeof(size_t)];

                size_t len = 2;
                push_to_vec(expected, local_buf, sizeof(size_t), &len);

                bool exists = true;
                const char *test = "test";
                push_to_vec(expected, local_buf, sizeof(bool), &exists);

                len = 4;
                push_to_vec(expected, local_buf, sizeof(size_t), &len);

                push_to_vec(expected, local_buf, len, test);

                exists = false;
                push_to_vec(expected, local_buf, sizeof(bool), &exists);

                REQUIRE(compare_serialized(serialized, expected));
            }

            THEN("It can deserialized and move constructed") {
                size_t pos = 0;
                auto d_sc = deserialize_strcolumn_and_alloc(serialized, pos);
                REQUIRE(d_sc);
                REQUIRE(d_sc->size() == sc.size());
                REQUIRE(d_sc->get(0) == sc.get(0));
                REQUIRE((!d_sc->get(1) && !sc.get(1)));
                REQUIRE(d_sc->equals(&sc));
            }
        }
    }
    
    GIVEN("A schema") {
        Schema schema("ISBF");
        WHEN("We serialize it"){
            std::vector<uint8_t> serialized = schema.serialize();
            THEN("It is properly serialized"){
                std::vector<uint8_t> expected;
                uint8_t local_buf[sizeof(size_t)];

                size_t width = 4;
                push_to_vec(expected, local_buf, sizeof(size_t), &width);

                char ctype = 'I';
                push_to_vec(expected, local_buf, sizeof(char), &ctype);

                ctype = 'S';
                push_to_vec(expected, local_buf, sizeof(char), &ctype);

                ctype = 'B';
                push_to_vec(expected, local_buf, sizeof(char), &ctype);

                ctype = 'F';
                push_to_vec(expected, local_buf, sizeof(char), &ctype);

                REQUIRE(compare_serialized(serialized, expected));
            }

            THEN("We can properly deserialize it") {
                size_t pos = 0;
                auto d_schema = deserialize_schema_and_alloc(serialized, pos);
                
                REQUIRE(d_schema->width() == schema.width());
                REQUIRE(d_schema->length() == schema.length());
                REQUIRE(d_schema->col_type(0) == schema.col_type(0));
                REQUIRE(d_schema->col_type(1) == schema.col_type(1));
                REQUIRE(d_schema->col_type(2) == schema.col_type(2));
                REQUIRE(d_schema->col_type(3) == schema.col_type(3));
                REQUIRE(d_schema->equals(&schema));
            }
        }
    }

    GIVEN("A 10000 Row DataFrame") {
        DataFrame df(std::make_unique<Schema>("ISFB"));
        generate_large_dataframe(df, 10000);
        THEN("We can serialize and deserialize it") {
            std::vector<uint8_t> serialized = df.serialize();

            size_t pos = 0;
            auto d_df = deserialize_dataframe_and_alloc(serialized, pos);

            REQUIRE(d_df);

            REQUIRE(d_df->ncols() == df.ncols());
            REQUIRE(d_df->nrows() == df.nrows());

            TestSumRower tsr_one;
            df.pmap(tsr_one);

            TestSumRower tsr_two;
            d_df->pmap(tsr_two);

            REQUIRE(tsr_one.get_sum() == tsr_two.get_sum());
        }
    }
}
