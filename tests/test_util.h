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

inline std::optional<int> generate_existing_int(){
    return std::optional<int>(rand() % 10000 * (rand() % 2 == 1 ? 1 : -1));
}

inline std::optional<int> generate_int(){
    if(rand() % 2 == 0) return std::nullopt;
    return generate_existing_int();
}

inline std::optional<bool> generate_existing_bool(){
    return std::optional<bool>(rand() % 2);
}

inline std::optional<bool> generate_bool(){
    if(rand() % 2 == 0) return std::nullopt;
    return generate_existing_bool();
}

inline std::optional<double> generate_existing_float() {
    return std::optional<double>((*generate_existing_int() * 1.0) / ((rand() % 5) * 1.0));
}

inline std::optional<double> generate_float(){
    if(rand() % 2 == 0) return std::nullopt;
    return generate_existing_float();
}

inline std::optional<std::string> generate_existing_string(){
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

inline std::optional<std::string> generate_string(){
    if(rand() % 2 == 0) return std::nullopt;
    return generate_existing_string();
}

inline bool generate_large_dataframe(DataFrame& df, size_t nrows){
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
