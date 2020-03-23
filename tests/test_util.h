#pragma once

#include <string>
#include <random>
#include <memory>
#include <optional>

#include "data/dataframe.h"
#include "data/schema.h"
#include "data/row.h"

/* #define SEED 102102 */

/* bool initalized = false; */

/* void init() { */
/*     if(initalized) return; */
/*     srand(SEED); */
/*     initalized = true; */
/* } */

std::optional<int> generate_int(){
    return std::optional<int>(rand() % 10000 * (rand() % 2 == 1 ? 1 : -1));
}

std::optional<bool> generate_bool(){
    return std::optional<bool>(rand() % 2);
}

std::optional<double> generate_float(){
    return std::optional<double>((*generate_int() * 1.0) / ((rand() % 5) * 1.0));
}

std::optional<std::string> generate_string(){
    char buffer[11]{};
    size_t length = rand() % 10;
    size_t i = 0;
    while(i < length) {
        bool upper = rand() % 2;
        buffer[i] = ((char) ((rand() % 26) + (upper ? 65 : 97)));
        ++i;
    }
    buffer[length] = '\0';
    return std::optional<std::string>(buffer);
}

bool generate_large_dataframe(DataFrame& df, size_t nrows){
    /* if(!initalized) init(); */
    const Schema& scm = df.get_schema();

    Row r(scm);
    for(size_t i = 0; i < nrows; ++i){
        r.set_index(i);
        for(size_t j = 0; j < r.width(); ++j){
            switch(scm.col_type(j)) {
                case 'I':
                    {
                        r.set(j, generate_int());
                        break;
                    } 
                case 'F':
                    {
                        r.set(j, generate_float());
                        break;
                    }
                case 'B':
                    {
                        r.set(j, generate_bool());
                        break;
                    }
                case 'S':
                    {
                        r.set(j, generate_string());
                        break;
                    }
                default:
                    return false;
            }
        }
        df.add_row(r);
    }
    return true;
}
