#include "loaded_data.hpp"
#include <cassert>
#include <chrono>
#include <cppweb_init.hpp>
#include <cppweb_server_component.hpp>
#include <dlfcn.h>
#include <sys/logger.hpp>
#include <thread>
#include <vector>

template <typename pfn_t>
pfn_t *get_func(void *so, std::string const &name, bool can_be_empty = false) {
  void *pfn = dlsym(so, name.c_str());

  if (pfn == nullptr && !can_be_empty) {
    sys::fatal_error("Error accessing function %s - %s", name.c_str(),
                     dlerror());
  }

  return reinterpret_cast<pfn_t *>(pfn);
}

#define GET_PFN(so, name)                                                      \
  auto pfn_##name = reinterpret_cast<typeof(name) *>(dlsym(so, #name))
#define CALL_PFN(name, ...)                                                    \
  pfn_##name ? pfn_##name(__VA_ARGS__) : (decltype(pfn_##name(__VA_ARGS__)))({})
#define CHECK_PFN(name) (pfn_##name != nullptr)

static std::vector<CPPWEB_ServerComponentDecl const *> SERVER_COMPONENTS;

void LoadedData::hot_reload() {
  auto so = so_file;
  close();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  load(so);
}

CPPWEB_ServerComponentDecl const *server_component_find(char const *name) {
  std::string name_s = name;
  for (auto c : SERVER_COMPONENTS)
    if (c->name == name_s)
      return c;
  return nullptr;
}

extern api::Api *THE_API;

InitServerData INIT_SERVER_DATA{
    .register_server_component =
        [](struct CPPWEB_ServerComponentDecl const *c) {
          SERVER_COMPONENTS.push_back(c);
        },
    .register_api = [](api::Api *api) { THE_API = api; }};

void LoadedData::load(std::string const &file) {
  so_file = file;
  so = dlopen(file.c_str(), RTLD_LAZY | RTLD_LOCAL);

  if (so == nullptr) {
    sys::fatal_error("Error loading library %s - %s", file.c_str(), dlerror());
  }

  dlerror();

  GET_PFN(so, init_server);
  CALL_PFN(init_server, &INIT_SERVER_DATA);

  instanciate_context =
      get_func<fn_instanciate_context_t>(so, "instanciate_context", true);
  if (instanciate_context)
    cleanup_context = get_func<fn_cleanup_context_t>(so, "cleanup_context");
}

void LoadedData::close() {
  GET_PFN(so, cleanup_server);
  CALL_PFN(cleanup_server);
  THE_API = nullptr;
  SERVER_COMPONENTS.clear();
  dlclose(so);
  so_file = "";
  so = nullptr;
  instanciate_context = nullptr;
  cleanup_context = nullptr;
}

LoadedData LOADED_SO{};
