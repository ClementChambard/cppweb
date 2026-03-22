#pragma once

#include <string>

struct Config {
  Config() = default;
  Config(std::string const &file_name);
  std::string cppweb_dir;
  std::string pages_dir;
  std::string pages_build_dir;
  std::string cpp_dir;
  std::string cpp_bin;
  std::string resources_dir;
  bool dev = false;
};

extern Config CONFIG;
