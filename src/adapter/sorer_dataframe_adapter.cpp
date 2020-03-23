#include <iostream>
#include <exception>
#include <cstring>

#include "libsorer.h"
#include "adapter/sorer_dataframe_adapter.h"
#include "data/schema.h"

namespace SorerDataframeAdapter {
    // anonymous namespace to simulate private
    namespace {
        std::unique_ptr<Schema> initialize_schema(SoRParser& parser) {
            // initialize schema
            std::string s_format;
            for(size_t i = 0; i < parser.ncols(); ++i){
                switch(parser.getColType(i)) {
                    case SoRType::BOOL:
                        s_format += 'B';
                        break;
                    case SoRType::INT:
                        s_format += 'I';
                        break;
                    case SoRType::FLOAT:
                        s_format += 'F';
                        break;
                    case SoRType::STRING:
                        s_format += 'S';
                        break;
                }
            }
            return std::make_unique<Schema>(s_format.c_str());
        }

        std::optional<int> parse_int(SoRParser& parser, size_t col, size_t row) {
            auto val = parser.getColIdx(col, row);
            if(val) {
                return std::optional<int>(std::stoi(*val));
            }
            return std::nullopt;
        }

        std::optional<bool> parse_bool(SoRParser& parser, size_t col, size_t row) {
            auto val = parser.getColIdx(col, row);
            if(val) {
                return std::optional<bool>(std::stoi(*val));
            }
            return std::nullopt;
        }

        std::optional<double> parse_float(SoRParser& parser, size_t col, size_t row) {
            auto val = parser.getColIdx(col, row);
            if(val) {
                return std::optional<double>(std::stod(*val));
            }
            return std::nullopt;
        }

        std::optional<std::string> parse_string(SoRParser& parser, size_t col, size_t row) {
            // already in the correct format
            return parser.getColIdx(col, row);
        }

        bool parse_and_fill_row(SoRParser& parser, size_t row, Row& row_obj){
            row_obj.set_index(row);
            for(size_t c = 0; c < parser.ncols(); ++c) {
                // ew, they use C++ exceptions to signal EOF
                try {
                    switch(parser.getColType(c)) {
                        case SoRType::INT:
                            row_obj.set(c, parse_int(parser, c, row));
                            break;
                        case SoRType::BOOL:
                            row_obj.set(c, parse_bool(parser, c, row));
                            break;
                        case SoRType::FLOAT:
                            row_obj.set(c, parse_float(parser, c, row));
                            break;
                        case SoRType::STRING:
                            row_obj.set(c, parse_string(parser, c, row));
                            break;
                    }
                } catch(std::exception& e) {
                    const char *e_str = e.what();
                    if(strcmp(e_str, "Invalid offset") != 0){
                        std::cerr <<"Unexpected Exception: " <<e_str <<std::endl;
                    }
                    return false;
                }
            }
            return true;
        }
    } // anonymous namespace

    std::shared_ptr<DataFrame> parse_file(const std::string& filename) {
        SoRParser parser;
        if(!parser.initialize(filename)) return nullptr;

        auto df = std::make_shared<DataFrame>(initialize_schema(parser));

        Row row(df->get_schema());
        size_t r = 0;
        while(parse_and_fill_row(parser, r++, row)){
            df->add_row(row);
        }
        return df;
    }
}
