#ifndef IG_SYS_LOGGER_HPP
#define IG_SYS_LOGGER_HPP

#include <cstdarg>
#include <cstdlib>
#include "../defines.hpp"

namespace sys {

namespace logger {

enum class Level : u64 {
  INFO = 0,
  WARN,
  ERROR,
  FATAL,
  LEVEL_COUNT,
};

void initialize();
void log_inner(Level lvl, char const *message, va_list args);

} // namespace logger

#define LOG_INNER(lvl, message)                                                \
  va_list args;                                                                \
  va_start(args, message);                                                     \
  logger::log_inner(lvl, message, args);                                       \
  va_end(args)

inline void info(char const *message, ...) {
  LOG_INNER(logger::Level::INFO, message);
}

inline void warn(char const *message, ...) {
  LOG_INNER(logger::Level::WARN, message);
}

inline void error(char const *message, ...) {
  LOG_INNER(logger::Level::ERROR, message);
}

inline void fatal_error(char const *message, ...) {
  LOG_INNER(logger::Level::FATAL, message);
  std::exit(EXIT_FAILURE);
}

#undef LOG_INNER

} // namespace sys

#endif // !IG_SYS_LOGGER_HPP
