#pragma once

#include <html/macros.hpp>

inline std::string cn(components::Params &params, std::string const &next) {
  std::string class_names = params.take_a<std::string>("class", "");
  if (class_names.size() > 0) {
    class_names = class_names + ' ' + next;
  } else {
    class_names = next;
  }

  return class_names;
}
