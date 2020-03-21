#pragma once

#include <memory>

#include "data/dataframe.h"

namespace SorerDataframeAdapter {
    std::shared_ptr<DataFrame> parse_file(const std::string& filename);
}
