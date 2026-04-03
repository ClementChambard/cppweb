#include "fs_helpers.hpp"
#include <filesystem>
#include <iostream>
#include <sys/env.hpp>

void cp_echo(std::string_view source, std::string_view dest) {
  std::filesystem::path destpath = dest;
  destpath += "/" + std::filesystem::path(source).filename().string();

  std::cout << "copying " << source << " to " << dest << '\n';
  std::filesystem::copy_file(source, destpath);
}

void cp_dir_echo(std::string_view source, std::string_view dest) {
  std::filesystem::path destpath = dest;
  destpath += "/" + std::filesystem::path(source).filename().string();

  std::cout << "copying directory " << source << " to " << dest << '\n';
  std::filesystem::copy(source, destpath,
                        std::filesystem::copy_options::recursive);
}

Config get_config() {
  if (!std::filesystem::exists("cppweb.conf")) {
    std::cerr << "Not a cppweb project\n";
    std::exit(EXIT_FAILURE);
  }
  return Config("cppweb.conf");
}

std::string get_cppweb_dir(bool in_config) {
  if (!in_config) {
    // find installed cppweb: TODO: installing
    // else:
    auto loc = sys::get_env_var("CPPWEB_DIR");
    if (loc == std::nullopt) {
      std::cerr << "Could not find cppweb directory (use env var CPPWEB_DIR)\n";
      std::exit(EXIT_FAILURE);
    }
    return *loc;
  }
  auto conf = get_config();
  return conf.cppweb.dir;
}
