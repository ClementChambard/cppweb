#pragma once

#include "request.hpp"
#include "response.hpp"
#include <functional>
#include <string>
#include <vector>

namespace http {

using EndpointFunc = std::function<Response(Request)>;

struct Route {
  Route() = default;
  Route(std::string_view rd, EndpointFunc f);
  std::string route_decl;
  EndpointFunc f;

  struct PathPart {
    std::string value;
    bool is_param;
  };

  std::vector<PathPart> route_parts;

  bool exec_if_match(Request &r, Response &res) const;

  static Route from_mangled(char const *mangled_endpoint, EndpointFunc fn);
};

struct ForwardedUrl {
  std::string base;
  EndpointFunc f;
  bool exec_if_match(Request &r, Response &res) const;
};

struct Router {

  Response process_request(Request &r) const;

  Router &route(char const *endpoint, Request::Kind k, EndpointFunc f);

  Router &page(char const *mangled_endpoint, EndpointFunc fn) {
    get_routes.push_back(Route::from_mangled(mangled_endpoint, fn));
    return *this;
  }
  Router &get(char const *endpoint, EndpointFunc fn) {
    return route(endpoint, Request::Kind::GET, fn);
  }
  Router &post(char const *endpoint, EndpointFunc fn) {
    return route(endpoint, Request::Kind::POST, fn);
  }
  Router &patch(char const *endpoint, EndpointFunc fn) {
    return route(endpoint, Request::Kind::PATCH, fn);
  }
  Router &put(char const *endpoint, EndpointFunc fn) {
    return route(endpoint, Request::Kind::PUT, fn);
  }
  Router &del(char const *endpoint, EndpointFunc fn) {
    return route(endpoint, Request::Kind::DELETE, fn);
  }
  Router &forward(char const *base_url, EndpointFunc fn) {
    forwarded.push_back({base_url, fn});
    return *this;
  }

  void clean();

  std::vector<Route> get_routes;
  std::vector<Route> post_routes;
  std::vector<Route> patch_routes;
  std::vector<Route> put_routes;
  std::vector<Route> delete_routes;
  std::vector<ForwardedUrl> forwarded;
};

} // namespace http
