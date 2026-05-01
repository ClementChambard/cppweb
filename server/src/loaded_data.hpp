#pragma once

#include <api/api.hpp>
#include <cppweb/server_rendering_context.hpp>

struct LoadedData {
  using fn_instanciate_context_t =
      void *(cppweb::ServerRenderingContext const &ctx);
  using fn_cleanup_context_t = void(void *);

  std::string so_file;
  void *so = nullptr;

  fn_instanciate_context_t *instanciate_context = nullptr;
  fn_cleanup_context_t *cleanup_context = nullptr;

  void load(std::string const &file);
  void hot_reload();
  void close();

  ~LoadedData() {
    if (so)
      close();
  }
};

extern LoadedData LOADED_SO;
