#include "components.hpp"

#include <dlfcn.h>
#include <iostream>

extern components::Map *REGISTERED_COMPONENTS;

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

  REGISTERED_COMPONENTS = get_registered_components();
  map = REGISTERED_COMPONENTS;
}

MapH::~MapH() {
  delete map;
  dlclose(so);
}
