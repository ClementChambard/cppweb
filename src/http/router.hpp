#ifndef IG_HTTP_ROUTER_HPP
#define IG_HTTP_ROUTER_HPP

#include "html/page.hpp"
#include "request.hpp"
#include "response.hpp"
#include <functional>
#include <string>
#include <vector>

namespace http {

using EndpointFunc = std::function<Response(Request)>;

struct Route {
  Route(std::string_view rd, EndpointFunc f);
  std::string route_decl;
  EndpointFunc f;
  std::vector<std::string_view> route_parts;
  std::vector<std::string_view> placeholder_names;
  bool exec_if_match(Request &r, Response &res) const;
};

struct Router {

  Response process_request(Request &r) const;

  Router &route(char const *endpoint, Request::Kind k, EndpointFunc f);

  Router &get(char const *endpoint, EndpointFunc fn) {
    return route(endpoint, Request::Kind::GET, fn);
  }
  Router &post(char const *endpoint, EndpointFunc fn) {
    return route(endpoint, Request::Kind::POST, fn);
  }
  Router &patch(char const *endpoint, EndpointFunc fn) {
    return route(endpoint, Request::Kind::PATCH, fn);
  }
  Router &page(char const *endpoint, html::Page &page) {
    return get(endpoint, html::page(page));
  }
  Router &page(char const *endpoint, html::Page &page,
               std::function<bool(Request &)> auth) {
    page.check_authorization = auth;
    return get(endpoint, html::page(page));
  }

  std::vector<Route> get_routes;
  std::vector<Route> post_routes;
  std::vector<Route> patch_routes;
};

} // namespace http

#endif // !IG_HTTP_ROUTER_HPP
