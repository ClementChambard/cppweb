#pragma once

#include <functional>
#include <http/request.hpp>
#include <http/response.hpp>
#include <http/ws_connection.hpp>
#include <string>
#include <vector>

using EndpointFunc = std::function<http::Response(http::Request)>;

using WSEndpointFunc = std::function<http::WSConnection *(http::Request)>;

struct Route {
  Route() = default;
  std::string route_decl;
  EndpointFunc f;

  struct PathPart {
    std::string value;
    bool is_param;
    bool is_catchall;
  };

  std::vector<PathPart> route_parts;

  bool exec_if_match(http::Request &r, http::Response &res) const;

  static Route from_mangled(char const *mangled_endpoint, EndpointFunc fn);
};

struct WSRoute {
  std::string endpoint;
  WSEndpointFunc f;
};

struct ForwardedUrl {
  std::string base;
  EndpointFunc f;
  bool exec_if_match(http::Request &r, http::Response &res) const;
};

struct Router {
  http::Response process_request(http::Request &r) const;
  http::WSConnection *maybe_process_ws(http::Request &r) const;

  Router &page(char const *mangled_endpoint, EndpointFunc fn) {
    get_routes.push_back(Route::from_mangled(mangled_endpoint, fn));
    return *this;
  }
  Router &forward(char const *base_url, EndpointFunc fn) {
    forwarded.push_back({base_url, fn});
    return *this;
  }
  Router &ws(char const *url, WSEndpointFunc fn) {
    ws_routes.push_back({url, fn});
    return *this;
  }
  Router &wo_ws(char const *url) {
    ws_routes.push_back(
        {url, [](http::Request const &) { return new http::WSConnection; }});
    return *this;
  }

  void clean();

  std::vector<WSRoute> ws_routes;
  std::vector<Route> get_routes;
  std::vector<ForwardedUrl> forwarded;
};
