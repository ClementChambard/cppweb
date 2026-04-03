#include "components.hpp"
#include "cppweb/logger.hpp"
#include "sys/logger.hpp"

#include <dlfcn.h>

extern components::Map *REGISTERED_COMPONENTS;
extern components::ServerMap *REGISTERED_SERVER_COMPONENTS;

MapH::MapH(std::string const &so_name) {
  // TODO: check actual location
  std::string libfullname = so_name;

  std::string func = "get_registered_components";

  using pfn_t = components::Map *();

  pfn_t *get_registered_components;

  so = dlopen(libfullname.c_str(), RTLD_LAZY | RTLD_LOCAL);

  if (so == nullptr) {
    logger::fatal_error("Error loading library %s - %s", libfullname.c_str(),
                        dlerror());
  }

  dlerror();

  void *pfn = dlsym(so, func.c_str());

  if (pfn == nullptr) {
    logger::fatal_error("Error accessing function %s - %s", func.c_str(),
                        dlerror());
  }

  get_registered_components = reinterpret_cast<pfn_t *>(pfn);

  using grsc_t = components::ServerMap *();
  pfn = dlsym(so, "get_registered_server_components");
  auto get_registered_server_components = reinterpret_cast<grsc_t *>(pfn);

  pfn = dlsym(so, "setup_logger_ext");
  auto setup_logger = reinterpret_cast<sys::setup_logger_t *>(pfn);
  setup_logger(logger::info, logger::warn, logger::error, logger::fatal_error,
               logger::log_extra);

  REGISTERED_COMPONENTS = get_registered_components();
  REGISTERED_SERVER_COMPONENTS = get_registered_server_components();
  map = REGISTERED_COMPONENTS;
  server_map = REGISTERED_SERVER_COMPONENTS;
}

MapH::~MapH() {
  using delete_api_t = void(void);
  auto delete_api = reinterpret_cast<delete_api_t *>(dlsym(so, "delete_api"));
  delete_api();
  dlclose(so);
}
