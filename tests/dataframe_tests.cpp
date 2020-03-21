#include "catch.hpp"
#include "test_util.h"
#include "test_rower.h"

#include "data/dataframe.h"
#include "data/schema.h"
#include "data/row.h"

#define ROW_CNT 100000

SCENARIO("Can construct and use dataframe"){
    GIVEN("A Constructed Schema") {
        auto s = std::make_unique<Schema>("IISSBBFF");
        REQUIRE(s->width() == 8);

        DataFrame df(std::move(s));
        WHEN("Data is added"){
            generate_large_dataframe(df, ROW_CNT);

            THEN("The dataframe length changes"){
                REQUIRE(df.nrows() == ROW_CNT);
            }
        }
        WHEN("The data is operated on in sequentially and in parallel results are the same") {
            // sum things single thread
            TestSumRower tsr_sequential;
            TestSumRower tsr_parallel;
            df.map(tsr_sequential);
            df.pmap(tsr_parallel);
            REQUIRE(tsr_sequential.get_sum() == tsr_parallel.get_sum());
        }
    }
}
