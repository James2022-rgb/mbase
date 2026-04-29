#pragma once

// c++ headers ------------------------------------------
#include <string_view>

namespace mbase {

void Trap();

void TrapIfWithMessage(bool condition, std::string_view message);

}
