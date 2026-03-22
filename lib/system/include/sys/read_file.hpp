#pragma once

#include <string>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace sys {

std::string read_text_file(char const *filename);
void write_text_file(char const *filename, std::string_view data);

struct FInfo {
  __time_t last_modified{};
  bool exists = false;
};

FInfo file_info(char const *filename);

} // namespace sys
