#include "catch.hpp"

#include "data/nullable_array.h"

SCENARIO("Can construct and use a nullable array") {
    GIVEN("A NullableArray of integers") {
        NullableArray<int> nai;
        WHEN("We add data"){
            // test push_back
            nai.push_back(0);
            nai.push_back(1);
            nai.push_back(std::nullopt);
            nai.push_back(3);
            nai.push_back(4);
            nai.push_back(std::nullopt);

            THEN("It is added in the correct order and we can access it"){
                // test get and assert push back worked as expected
                REQUIRE(nai.size() == 6);
                for(size_t i = 0; i < nai.size(); ++i) {
                    if(i != 2 && i != 5) {
                        REQUIRE(nai.get(i) == std::optional<int>(i));
                    } else {
                        REQUIRE(nai.get(i) == std::nullopt);
                    }
                }
            }

            THEN("We can change data at valid indices and remove data at valid indices") {
                REQUIRE(nai.get(2) == std::nullopt);
                REQUIRE(nai.get(3) == std::optional<int>(3));
                // test set
                nai.set(2, 2);
                nai.set(3, std::nullopt);
                // size hasn't changed
                REQUIRE(nai.size() == 6);
                REQUIRE(nai.get(2) == std::optional<int>(2));
                REQUIRE(nai.get(3) == std::nullopt);

                // test pop
                REQUIRE(nai.pop(3) == std::nullopt);
                REQUIRE(nai.size() == 5);
                REQUIRE(nai.pop(2) == std::optional<int>(2));
                REQUIRE(nai.size() == 4);
            }

            THEN("We can clone it and they are equal") {
                std::shared_ptr<Object> new_nai = nai.clone();
                REQUIRE(nai.equals(new_nai.get()));
                REQUIRE(nai.hash() == new_nai->hash());
            }

            THEN("We can serialize and deserialize") {
                std::vector<uint8_t> serialized = nai.serialize();
                size_t pos = 0;
                NullableArray<int> new_nai = NullableArray<int>::deserialize(serialized, pos);
                REQUIRE(nai == new_nai);
                REQUIRE(nai.equals(&new_nai));
                REQUIRE(nai.hash() == new_nai.hash());
            }
        }
    }
}

SCENARIO("Given two NullableArrays storing the same data") {
    GIVEN("Two NullableArrays of Strings storing the same strings and missing values") {
        NullableArray<std::string> nas1;
        nas1.push_back(std::string("hello"));
        nas1.push_back(std::nullopt);
        nas1.push_back(std::string("test"));

        NullableArray<std::string> nas2;
        nas2.push_back(std::string("hello"));
        nas2.push_back(std::nullopt);
        nas2.push_back(std::string("test"));

        THEN("They are equal and have the same hash") {
            REQUIRE(nas1 == nas2);
            REQUIRE(nas1.equals(&nas2));
            REQUIRE(nas1.hash() == nas2.hash());
        }
    }
}
