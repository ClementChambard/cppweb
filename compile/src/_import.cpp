#include "components.hpp"
#include "cppweb/logger.hpp"
#include "sys/logger.hpp"

#include <dlfcn.h>

extern components::Map *REGISTERED_COMPONENTS;
extern components::ServerMap *REGISTERED_SERVER_COMPONENTS;

static MapH *m;

void init() {
  std::string func = "get_registered_components";

  using pfn_t = components::Map *();

  pfn_t *get_registered_components;

  m->so = dlopen(m->lib_full_name.c_str(), RTLD_LAZY | RTLD_LOCAL);

  if (m->so == nullptr) {
    logger::fatal_error("Error loading library %s - %s",
                        m->lib_full_name.c_str(), dlerror());
  }

  dlerror();

  void *pfn = dlsym(m->so, func.c_str());

  if (pfn == nullptr) {
    logger::fatal_error("Error accessing function %s - %s", func.c_str(),
                        dlerror());
  }

  get_registered_components = reinterpret_cast<pfn_t *>(pfn);

  using grsc_t = components::ServerMap *();
  pfn = dlsym(m->so, "get_registered_server_components");
  auto get_registered_server_components = reinterpret_cast<grsc_t *>(pfn);

  pfn = dlsym(m->so, "setup_logger_ext");
  auto setup_logger = reinterpret_cast<sys::setup_logger_t *>(pfn);
  setup_logger(logger::info, logger::warn, logger::error, logger::fatal_error,
               logger::log_extra);

  REGISTERED_COMPONENTS = get_registered_components();
  REGISTERED_SERVER_COMPONENTS = get_registered_server_components();
  m->map = REGISTERED_COMPONENTS;
  m->server_map = REGISTERED_SERVER_COMPONENTS;
}

MapH::MapH(std::string const &so_name) {
  m = this;
  lib_full_name = so_name;
  init();
}

using delete_api_t = void(void);

MapH::~MapH() {
  auto delete_api = reinterpret_cast<delete_api_t *>(dlsym(so, "delete_api"));
  delete_api();
  dlclose(so);
}

void MapH::hot_reload() {
  auto delete_api =
      reinterpret_cast<delete_api_t *>(dlsym(m->so, "delete_api"));
  delete_api();
  dlclose(m->so);
  init();
}
