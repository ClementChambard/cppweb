#pragma once

#include "handler.hpp"
#include <http/request.hpp>
#include <string>
#include <vector>

namespace api {

struct Endpoint {
  Endpoint() = default;

  struct Route {
    http::Method method;
    std::string subpath;
    HandlerChain handlers;
  };

  std::string base_path;
  std::vector<Endpoint> sub_endpoints;
  std::vector<Route> routes;

  Endpoint &sub(std::string const &route, Endpoint const &ep) {
    sub_endpoints.push_back(ep);
    sub_endpoints.back().base_path = route;
    return *this;
  }

  // clang-format off
  template <typename ...Args>
  Endpoint &get(std::string const &subpath, Args ...args) { return route(http::Method::GET, subpath, args...); }
  template <typename ...Args>
  Endpoint &post(std::string const &subpath, Args ...args) { return route(http::Method::POST, subpath, args...); }
  template <typename ...Args>
  Endpoint &patch(std::string const &subpath, Args ...args) { return route(http::Method::PATCH, subpath, args...); }
  template <typename ...Args>
  Endpoint &put(std::string const &subpath, Args ...args) { return route(http::Method::PUT, subpath, args...); }
  template <typename ...Args>
  Endpoint &del(std::string const &subpath, Args ...args) { return route(http::Method::DELETE, subpath, args...); }
  // clang-format on

  Endpoint &register_at_root(std::string const &path);

private:
  template <typename... Args>
  Endpoint &route(http::Method method, std::string const &subpath,
                  Args... args) {
    routes.push_back({method, subpath, {args...}});
    return *this;
  }
};

#define __DECLARE_API_CONCAT(a, b) a##b
#define __ENDPOINT_NAME(b) __DECLARE_API_CONCAT(api_endpoint_of_this_file, b)

#define DECLARE_API static auto __ENDPOINT_NAME(__LINE__) = api::Endpoint

} // namespace api
