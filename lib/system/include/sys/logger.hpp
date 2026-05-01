#pragma once

#include <cstdarg>
#include <defines.hpp>

namespace sys {

enum class LogLevel : u64 {
  EXTRA = 0,
  INFO,
  WARN,
  ERROR,
  FATAL,
  LEVEL_COUNT,
};

void load_logger_config();
void log_inner(LogLevel lvl, char const *extra_name, char const *message,
               va_list args);

void info(char const *message, ...);
void warn(char const *message, ...);
void error(char const *message, ...);
void fatal_error(char const *message, ...);
void log_extra(const char *extra, const char *message, ...);
} // namespace sys
