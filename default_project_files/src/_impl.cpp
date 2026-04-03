#define SYS_LOGGER_IMPL
#include <sys/logger.hpp>
#define HTML_COMPONENTS_IMPL
#include <html/components.hpp>

static struct DeleteOnClose {
  ~DeleteOnClose() {
    delete REGISTERED_COMPONENTS;
    delete REGISTERED_SERVER_COMPONENTS;
    REGISTERED_COMPONENTS = nullptr;
    REGISTERED_SERVER_COMPONENTS = nullptr;
  }
} DOC;
