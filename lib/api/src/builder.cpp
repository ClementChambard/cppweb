#include "route.hpp"
#include <api.hpp>
#include <builder.hpp>
#include <cassert>

extern "C" api::Api *get_api();

namespace api {

RoutePath parse_path(std::string const &path) {
  RoutePath out;
  assert(path[0] == '/');

  size_t pos = 1;
  while (pos < path.size()) {
    size_t end_pos = path.find('/', pos);
    auto sub = path.substr(pos, end_pos - pos);
    if (sub.size() != 0) {
      bool arg = false;
      if (sub.starts_with(':')) {
        arg = true;
        sub = sub.substr(1);
      }
      out.push_back({.value = sub, .is_param = arg});
    }
    if (end_pos == std::string::npos)
      break;
    pos = end_pos + 1;
  }

  return out;
}

Endpoint &Endpoint::register_at_root(std::string const &path) {
  auto api = get_api();

  base_path = path;
  if (!base_path.starts_with("/")) {
    base_path.insert(0, "/");
  }

  for (auto const &r : routes) {
    std::string full_path = r.subpath;
    if (!full_path.starts_with("/")) {
      full_path.insert(0, "/");
    }
    if (base_path != "/")
      full_path = base_path + full_path;
    if (full_path.ends_with('/') && full_path != "/")
      full_path.pop_back();

    api->routes.push_back({
        .method = r.method,
        .path = parse_path(full_path),
        .handlers = r.handlers,
    });
  }

  for (auto &s : sub_endpoints) {
    if (!s.base_path.starts_with("/")) {
      s.base_path.insert(0, "/");
    }

    if (base_path != "/")
      s.base_path = base_path + s.base_path;
    if (s.base_path.ends_with('/'))
      s.base_path.pop_back();

    s.register_at_root(s.base_path);
  }

  return *this;
}

} // namespace api
