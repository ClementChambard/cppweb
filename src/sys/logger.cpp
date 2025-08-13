#include "logger.hpp"
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#define LOG_FILE_NAME "console.log"

namespace sys::logger {

static i32 log_file_fd = -1;

void shutdown() {
  if (log_file_fd >= 0) {
    close(log_file_fd);
  }
}

void initialize() {
  log_file_fd = open(LOG_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  std::atexit(shutdown);
}

void append_to_log_file(char const *message) {
  u64 length = std::strlen(message);
  write(log_file_fd, message, length);
}

void print_colorized(Level lvl, char const *message) {
  (void)lvl;
  // TODO: if colors supported, colorize output.
  // TODO: if is_error, use stderr
  std::printf("%s", message);
}

void log_inner(Level lvl, const char *message, va_list args) {
  static char const *const level_strings[u64(Level::LEVEL_COUNT)] = {
      "[INFO ]: ", "[WARN ]: ", "[ERROR]: ", "[FATAL]: "};
  // bool is_error = u64(lvl) >= u64(Level::ERROR);

  // TODO: how to make it better ?
  char message_buffer[16000] = {0};
  vsnprintf(message_buffer, sizeof(message_buffer), message, args);
  char out_message[32000] = {0};
  snprintf(out_message, sizeof(out_message), "%s%s\n", level_strings[u64(lvl)], message_buffer);

  print_colorized(lvl, out_message);

  if (log_file_fd >= 0) {
    append_to_log_file(out_message);
  }
}

} // namespace sys::logger
