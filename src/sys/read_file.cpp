#include "read_file.hpp"
#include "logger.hpp"
#include <fstream>

namespace sys {

std::string read_text_file(char const *filename) {
  std::ifstream f(filename);
  if (f.bad()) {
    error("   * INVALID FILE: %s", filename);
    return "";
  }
  info("   * READING %s", filename);
  f.seekg(0, std::ios::end);
  auto size = f.tellg();
  f.seekg(0, std::ios::beg);
  std::string s;
  s.resize(size, '\0');
  f.read(s.data(), size);
  return s;
}

void write_text_file(char const *filename, std::string_view data) {
  std::ofstream f(filename);
  if (f.bad()) {
    error("   * INVALID FILE: %s", filename);
    return;
  }
  info("   * WRITING %s", filename);
  f.write(data.data(), data.size());
}

FInfo file_info(char const *filename) {
  FInfo res{0, false};
  struct stat result;
  if (stat(filename, &result) == 0) {
    res.last_modified = result.st_mtime;
    res.exists = true;
  }
  return res;
}

} // namespace sys
