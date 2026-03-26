#include "components.hpp"

#include <dlfcn.h>
#include <iostream>

extern components::Map *REGISTERED_COMPONENTS;
extern components::ServerMap *REGISTERED_SERVER_COMPONENTS;

MapH::MapH(std::string const &so_name) {
  // TODO: check actual location
  std::string libfullname = so_name;

  std::string func = "get_registered_components";

  using pfn_t = components::Map *();

  pfn_t *get_registered_components;

  so = dlopen(libfullname.c_str(), RTLD_LAZY);

  if (so == nullptr) {
    std::cerr << "Error loading library " << libfullname << " - " << dlerror()
              << '\n';
    std::exit(EXIT_FAILURE);
  }

  dlerror();

  void *pfn = dlsym(so, func.c_str());

  if (pfn == nullptr) {
    std::cerr << "Error accessing function " << func << " - " << dlerror()
              << '\n';
    std::exit(EXIT_FAILURE);
  }

  get_registered_components = reinterpret_cast<pfn_t *>(pfn);

  using grsc_t = components::ServerMap *();
  pfn = dlsym(so, "get_registered_server_components");
  auto get_registered_server_components = reinterpret_cast<grsc_t *>(pfn);

  REGISTERED_COMPONENTS = get_registered_components();
  REGISTERED_SERVER_COMPONENTS = get_registered_server_components();
  map = REGISTERED_COMPONENTS;
  server_map = REGISTERED_SERVER_COMPONENTS;
}

MapH::~MapH() {
  using delete_api_t = void(void);
  auto delete_api = reinterpret_cast<delete_api_t *>(dlsym(so, "delete_api"));
  delete_api();
  delete map;
  delete server_map;
  dlclose(so);
}
