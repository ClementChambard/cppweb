#include <api/api.hpp>

static api::Api *API;

extern "C" api::Api *get_api() {
  if (API == nullptr)
    API = new api::Api;
  return API;
}

extern "C" void init_api() {}

extern "C" void delete_api() {
  delete API;
  API = nullptr;
}
