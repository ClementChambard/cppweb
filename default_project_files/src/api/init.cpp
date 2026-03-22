#include <api/api.hpp>

api::Api *API;

extern "C" api::Api *get_api() {
  if (API == nullptr)
    API = new api::Api;
  return API;
}

extern "C" void init_api() {}
