#pragma once

namespace sys {

using log_fn_t = void(char const *message, ...);
using log_extra_fn_t = void(char const *extra, char const *message, ...);

extern log_fn_t *info;
extern log_fn_t *warn;
extern log_fn_t *error;
extern log_fn_t *fatal_error;
extern log_extra_fn_t *log_extra;

#define safe_log(fn, ...)                                                      \
  if (sys::fn != nullptr)                                                      \
  sys::fn(__VA_ARGS__)

void setup_logger(log_fn_t *info, log_fn_t *warn, log_fn_t *error,
                  log_fn_t *fatal_error, log_extra_fn_t *log_extra);

using setup_logger_t = void(log_fn_t *info, log_fn_t *warn, log_fn_t *error,
                            log_fn_t *fatal_error, log_extra_fn_t *log_extra);

} // namespace sys

#ifdef SYS_LOGGER_IMPL
extern "C" void setup_logger_ext(sys::log_fn_t *info, sys::log_fn_t *warn,
                                 sys::log_fn_t *error,
                                 sys::log_fn_t *fatal_error,
                                 sys::log_extra_fn_t *log_extra) {
  sys::info = info;
  sys::warn = warn;
  sys::error = error;
  sys::fatal_error = fatal_error;
  sys::log_extra = log_extra;
}
#endif
