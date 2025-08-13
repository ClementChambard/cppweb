#ifndef IG_SYS_ENV_HPP
#define IG_SYS_ENV_HPP

#include <cstdlib>
#include <string>
namespace sys {

inline std::string get_env_var(char const *key, char const *default_val) {
  char *val = std::getenv(key);
  return val == NULL ? default_val : val;
}

} // namespace sys

#endif // !IG_SYS_ENV_HPP
