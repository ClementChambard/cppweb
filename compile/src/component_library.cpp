#include "component_library.hpp"
#include "builtin_components.hpp"
#include "cppweb_init.hpp"

#include <dlfcn.h>
#include <string>
#include <sys/logger.hpp>

#define GET_PFN(so, name)                                                      \
  auto pfn_##name = reinterpret_cast<typeof(name) *>(dlsym(so, #name))
#define CALL_PFN(name, ...)                                                    \
  pfn_##name ? pfn_##name(__VA_ARGS__) : (decltype(pfn_##name(__VA_ARGS__)))({})
#define CHECK_PFN(name) (pfn_##name != nullptr)

static std::vector<CPPWEB_StaticComponentDecl const *> LOADED_COMPONENTS;
static std::vector<char const *> SERVER_COMPONENTS;

InitCompilerData COMPILER_INIT{.register_static_component =
                                   [](CPPWEB_StaticComponentDecl const *cmp) {
                                     // TODO: error if multiple definition
                                     LOADED_COMPONENTS.push_back(cmp);
                                   },
                               .register_server_component =
                                   [](char const *name) {
                                     // TODO: error if multiple definition
                                     SERVER_COMPONENTS.push_back(name);
                                   }};

ComponentLibrary::ComponentLibrary(std::string const &so_name) {
  LOADED_COMPONENTS.clear();
  SERVER_COMPONENTS.clear();

  LOADED_COMPONENTS.push_back(&CPPWEB_UseJs_DECL);
  LOADED_COMPONENTS.push_back(&CPPWEB_UseCss_DECL);
  LOADED_COMPONENTS.push_back(&CPPWEB_PageTitle_DECL);
  LOADED_COMPONENTS.push_back(&CPPWEB_Children_DECL);

  so = dlopen(so_name.c_str(), RTLD_LAZY | RTLD_LOCAL);

  if (so == nullptr) {
    sys::fatal_error("Error loading library %s - %s", so_name.c_str(),
                     dlerror());
  }

  GET_PFN(so, init_compiler);
  if (!CHECK_PFN(init_compiler)) {
    sys::fatal_error(
        "Error in library %s - entry point function 'init_compiler' not found",
        so_name.c_str());
  }
  CALL_PFN(init_compiler, &COMPILER_INIT);
}

ComponentLibrary::~ComponentLibrary() {
  GET_PFN(so, cleanup_compiler);
  CALL_PFN(cleanup_compiler);

  dlclose(so);
}

CPPWEB_StaticComponentDecl const *ComponentLibrary::find(char const *name) {
  auto n = std::string(name);
  for (auto c : LOADED_COMPONENTS)
    if (c->name == n)
      return c;
  for (auto c : SERVER_COMPONENTS)
    if (c == n)
      return &CPPWEB_ServerComponent_DECL;
  return nullptr;
}
