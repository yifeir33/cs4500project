#include "catch.hpp"

#include "adapter/sorer_dataframe_adapter.h"
#include "data/dataframe.h"

SCENARIO("Can use Sorer library to construct dataframe"){
    GIVEN("A SOR file with a \'BIS\' Schema and 3 Rows"){
        THEN("We can construct a dataframe from it"){
            std::string fn("/home/neil/school/software_development/cs4500project/tests/basic.sor");
            auto df = SorerDataframeAdapter::parse_file(fn);
            REQUIRE(df);

            REQUIRE(df->get_schema().width() == 3);
            REQUIRE(df->get_schema().col_type(0) == 'B');
            REQUIRE(df->get_schema().col_type(1) == 'I');
            REQUIRE(df->get_schema().col_type(2) == 'S');

            REQUIRE(df->ncols() == 3);
            REQUIRE(df->nrows() == 3);

            REQUIRE((df->get_bool(0, 0).has_value() && *(df->get_bool(0, 0)) == false));
            REQUIRE((df->get_int(1, 0).has_value() && *(df->get_int(1, 0)) == 23));
            REQUIRE((df->get_string(2, 0).lock() && *(df->get_string(2, 0).lock()) == std::string("hi")));

            REQUIRE((df->get_bool(0, 1).has_value() && *(df->get_bool(0, 1)) == true));
            REQUIRE((df->get_int(1, 1).has_value() && *(df->get_int(1, 1)) == 12));
            REQUIRE((!df->get_string(2, 1).lock()));

            REQUIRE((df->get_bool(0, 2).has_value() && *(df->get_bool(0, 2)) == true));
            REQUIRE((df->get_int(1, 2).has_value() && *(df->get_int(1, 2)) == 1));
            REQUIRE((!df->get_string(2, 2).lock()));
         }
    }
}
