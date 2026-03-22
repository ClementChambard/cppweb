#pragma once

#include <string>
#include <sys/read_file.hpp>

namespace http {

struct Resource {
  std::string file_name;
  std::string resource_type;
  std::string etag;
  std::string cached_contents;
  sys::FInfo info;
  bool gzipped = false;
  bool cache_contents = false;

  Resource() = default;
  Resource(std::string const &file_name);

  static Resource const &get(std::string const &name, bool cache_contents = false);

  std::string get_res_contents() const;
};

} // namespace http
