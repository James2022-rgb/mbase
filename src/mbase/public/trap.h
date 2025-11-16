#pragma once

// c++ headers ------------------------------------------
#include <string_view>

// external headers -------------------------------------
#include "spdlog/fmt/fmt.h"

namespace mbase {

void Trap();

void TrapIfWithMessage(bool condition, std::string_view message);

}
