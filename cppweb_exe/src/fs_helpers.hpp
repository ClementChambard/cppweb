#pragma once

#include <cppweb/config.hpp>
#include <string_view>

void cp_echo(std::string_view source, std::string_view dest);
void cp_dir_echo(std::string_view source, std::string_view dest);

Config get_config();
std::string get_cppweb_dir(bool in_config = true);
