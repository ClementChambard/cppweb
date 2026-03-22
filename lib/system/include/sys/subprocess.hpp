#pragma once

#include <span>

namespace sys {

void subprocess_run(std::span<char const *> args);

} // namespace sys
