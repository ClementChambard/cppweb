#include <logger.hpp>

namespace sys {

log_fn_t *info = nullptr;
log_fn_t *warn = nullptr;
log_fn_t *error = nullptr;
log_fn_t *fatal_error = nullptr;
log_extra_fn_t *log_extra = nullptr;

void setup_logger(log_fn_t *info, log_fn_t *warn, log_fn_t *error,
                  log_fn_t *fatal_error, log_extra_fn_t *log_extra) {
  sys::info = info;
  sys::warn = warn;
  sys::error = error;
  sys::fatal_error = fatal_error;
  sys::log_extra = log_extra;
}
} // namespace sys
