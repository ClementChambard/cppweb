#include "logger.hpp"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>

#include <iomanip>

#define LOG_FILE_NAME "console.log"

namespace sys::logger {

static i32 log_file_fd = -1;

void shutdown() {
  if (log_file_fd >= 0) {
    close(log_file_fd);
  }
}

void initialize() {
  log_file_fd = open(LOG_FILE_NAME, O_WRONLY | O_CREAT | O_APPEND, 0644);
  std::atexit(shutdown);
}

void append_to_log_file(char const *message) {
  u64 length = std::strlen(message);
  write(log_file_fd, message, length);
}

void print_colorized(Level lvl, char const *message) {
  // TODO: only if colors supported, colorize output.
  // TODO: if is_error, use stderr
  if (lvl == Level::WARN) {
    std::printf("\x1b[33m");
  }
  if (lvl == Level::ERROR) {
    std::printf("\x1b[31m");
  }
  if (lvl == Level::FATAL) {
    std::printf("\x1b[1m");
  }
  std::printf("%s", message);
  std::printf("\x1b[0m");
}

void log_inner(Level lvl, const char *message, va_list args) {

  
  static char const *const level_strings[u64(Level::LEVEL_COUNT)] = {
      "[INFO ]: ", "[WARN ]: ", "[ERROR]: ", "[FATAL]: "};
  // bool is_error = u64(lvl) >= u64(Level::ERROR);

  // TODO: how to make it better ?
  char message_buffer[16000] = {0};
  vsnprintf(message_buffer, sizeof(message_buffer), message, args);

  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);

  std::ostringstream oss;
  oss << std::put_time(&tm, "%d-%m-%Y %H:%M:%S ") << level_strings[u64(lvl)] << message_buffer << '\n';
  auto out_message = oss.str();

  print_colorized(lvl, out_message.c_str());

  if (log_file_fd >= 0) {
    append_to_log_file(out_message.c_str());
  }
}

} // namespace sys::logger
