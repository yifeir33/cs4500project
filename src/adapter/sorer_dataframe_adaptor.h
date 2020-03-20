#pragma once

#include <memory>

#include "libsorer.h"
#include "data/dataframe.h"

namespace SorerDataframeAdaptor {
    std::shared_ptr<DataFrame> parse_file(const std::string& filename);
}
