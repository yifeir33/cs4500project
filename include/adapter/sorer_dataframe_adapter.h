#pragma once

#include <memory>

#include "data/dataframe.h"

/** Namespace which wraps the other group's Sorer implemntation in
 * a function which creates the DataFrame object used by our project. */
namespace SorerDataframeAdapter {
    /** Parses a file in SOR format using the other group's Sorer implementation
     * into our dataframe object. */
    std::shared_ptr<DataFrame> parse_file(const std::string& filename);
}
