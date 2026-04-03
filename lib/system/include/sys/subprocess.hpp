#pragma once

#include <span>
#include <string>
#include <vector>

namespace sys {

void subprocess_run(std::span<char const *> args);

void export_env_var(std::string_view name, std::string_view value);

int exe_run(std::string const &exe_name, std::vector<std::string> args = {},
            bool fork = true, bool wait = true);

} // namespace sys
