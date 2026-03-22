#pragma once

#include <cstdlib>
#include <string>
namespace sys {

inline std::string get_env_var(char const *key, char const *default_val) {
  char *val = std::getenv(key);
  return val == NULL ? default_val : val;
}

} // namespace sys
