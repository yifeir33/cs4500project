#include "catch.hpp"
#include "test_util.h"
#include "test_rower.h"

#include "src/data/dataframe.h"
#include "src/data/schema.h"
#include "src/data/row.h"

#define ROW_CNT 100000

SCENARIO("Can construct and use dataframe"){
    GIVEN("A Constructed Schema") {
        Schema s("IISSBBFF");
        REQUIRE(s.width() == 8);

        Dataframe df(s);
        WHEN("Data is added"){
            generate_large_dataframe(df, ROW_CNT);

            THEN("The dataframe length changes"){
                REQUIRE(df.length() == ROW_CNT);
            }
        }
        WHEN("The data is operated on in sequentially and in parallel results are the same") {
            // sum things single thread
            TestSumRower tsr_sequential;
            TestSumRower tsr_parallel;
            df.map(tsr_sequential);
            df.pmap(tsr_parallel);
            REQUIRE(tsr_sequential.get_sum() = tsr_parallel.get_sum());
        }
    }
}
