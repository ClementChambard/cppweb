#pragma once

#include <functional>
#include <http/request.hpp>
#include <http/response.hpp>
#include "route.hpp"
#include <string>
#include <vector>
namespace api {

struct Api {
  std::string base_url = "/api";

  http::Response dispatch_request(std::string route, http::Request const &req);

  std::vector<Route> routes;

  std::function<http::Response(http::Request)> get_func();
};

} // namespace api
