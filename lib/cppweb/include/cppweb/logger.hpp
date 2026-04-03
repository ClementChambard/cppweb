#pragma once

#include "config.hpp"
#include <cstdarg>
#include <defines.hpp>

namespace logger {

enum class Level : u64 {
  EXTRA = 0,
  INFO,
  WARN,
  ERROR,
  FATAL,
  LEVEL_COUNT,
};

void set_config(Config::logger_t const &config);
void log_inner(Level lvl, char const *extra_name, char const *message,
               va_list args);

void info(char const *message, ...);
void warn(char const *message, ...);
void error(char const *message, ...);
void fatal_error(char const *message, ...);
void log_extra(const char *extra, const char *message, ...);

#undef LOG_INNER

} // namespace logger
