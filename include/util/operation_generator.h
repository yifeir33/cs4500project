#pragma once

#include <memory>
#include <vector>

#include "data/rower.h"

namespace OperationGenerator {
    std::shared_ptr<Rower> operation(size_t opcode);

    std::vector<uint8_t> operation_result(size_t opcode, std::shared_ptr<Rower>);
}

