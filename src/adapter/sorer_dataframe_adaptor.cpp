
// TODO: prepend w/ adaptor/
#include "sorer_dataframe_adaptor.h"
#include "data/schema.h"

namespace SorerDataframeAdaptor {
    std::shared_ptr<DataFrame> parse_file(const std::string& filename) {
        SoRParser parser;
        parser.initialize(filename);
        std::string s_format;
        // initialize schema
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
        auto schema = std::make_shared<Schema>(s_format.c_str());
        auto df = std::make_shared<DataFrame>(schema);
        // we can no longer externally edit schema
        // TODO pull data and parse values
    }
}
