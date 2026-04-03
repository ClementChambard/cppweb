#pragma once

#include <cstdlib>
#include <optional>
#include <string>

namespace sys {

inline std::string get_env_var(char const *key, char const *default_val) {
  char *val = std::getenv(key);
  return val == NULL ? default_val : val;
}

inline std::optional<std::string> get_env_var(char const *key) {
  if (char const *val = std::getenv(key)) {
    return val;
  }
  return std::nullopt;
}

} // namespace sys
