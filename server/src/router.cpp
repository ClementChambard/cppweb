#include "router.hpp"
#include "http/ws_connection.hpp"
#include <cassert>
#include <http/request.hpp>
#include <http/resource.hpp>
#include <http/response.hpp>
#include <http/url_params.hpp>
#include <sys/read_file.hpp>

Route Route::from_mangled(char const *mangled_endpoint, EndpointFunc fn) {
  Route r{};
  r.route_decl = mangled_endpoint;
  r.f = fn;

  std::string_view cursor = mangled_endpoint;
  cursor = cursor.substr(1);
  while (cursor != ".html") {
    u32 count = 0;
    bool p = false;
    bool a = false;
    if (cursor[0] == 'p') {
      p = true;
      cursor = cursor.substr(1);
    }
    if (cursor[0] == 'a') {
      a = true;
      cursor = cursor.substr(1);
    }
    while (cursor[0] >= '0' && cursor[0] <= '9') {
      count *= 10;
      count += cursor[0] - '0';
      cursor = cursor.substr(1);
    }
    r.route_parts.emplace_back(std::string(cursor.substr(0, count)), p, a);
    cursor = cursor.substr(count);
  }

  return r;
}

std::vector<std::string_view> split_request_route(std::string const &r) {
  assert(r.starts_with('/'));
  if (r == "/")
    return {};
  std::vector<std::string_view> out;
  std::string_view cursor = r;
  cursor = cursor.substr(1);
  while (true) {
    auto pos = cursor.find('/');
    out.push_back(cursor.substr(0, pos));
    if (pos == std::string_view::npos || pos == cursor.size() - 1) {
      break;
    }
    cursor = cursor.substr(pos + 1);
  }
  return out;
}

bool check_match(http::Request &r, Route const &route,
                 http::UrlParams &route_params) {
  auto request_route = split_request_route(r.endpoint);
  // TODO: better catchall handling
  if (request_route.size() != route.route_parts.size() &&
      (route.route_parts.size() == 0 || !route.route_parts.back().is_catchall))
    return false;
  for (u32 i = 0; i < route.route_parts.size(); i++) {
    if (i >= request_route.size()) {
      if (route.route_parts[i].is_catchall) {
        route_params.insert(std::make_pair(route.route_parts[i].value, "/"));
        break;
      }
      return false;
    }
    if (route.route_parts[i].is_param) {
      route_params.insert(std::make_pair(route.route_parts[i].value,
                                         http::url_decode(request_route[i])));
    } else if (route.route_parts[i].is_catchall) {
      std::string const &param_name = route.route_parts[i].value;
      std::string out;
      while (i < request_route.size()) {
        out += '/';
        out += http::url_decode(request_route[i]);
        i++;
      }
      route_params.insert(std::make_pair(param_name, out));
      break;
    } else if (route.route_parts[i].value != request_route[i]) {
      return false;
    }
  }
  return true;
}

bool Route::exec_if_match(http::Request &r, http::Response &res) const {
  http::UrlParams route_params;
  if (!check_match(r, *this, route_params))
    return false;

  for (auto [k, v] : route_params) {
    r.params[k] = v;
  }

  res = this->f(r);
  return true;
}

bool ForwardedUrl::exec_if_match(http::Request &r, http::Response &res) const {
  bool starts_with_route = r.endpoint.starts_with(base + '/');
  bool is_route = r.endpoint == base;
  bool is_route_with_query = r.endpoint.starts_with(base + '?') &&
                             !r.endpoint.substr(base.size() + 1).contains('/');
  if (starts_with_route || is_route || is_route_with_query) {
    res = f(r);
    return true;
  }
  return false;
}

http::Response get_resource(http::Request r) {
  http::Resource const &res = http::Resource::get(r.endpoint);
  if (!res.info.exists) {
    return http::Response::not_found();
  }
  if (auto header_it = r.headers.find("If-None-Match");
      header_it != r.headers.end()) {
    if (res.etag == header_it->second) {
      return http::Response::Builder()
          .code(304)
          .header("etag", res.etag.c_str())
          .close()
          .build();
    }
  }
  std::string file_content = res.get_res_contents();
  auto b = http::Response::Builder()
               .code(200)
               .body(res.resource_type, file_content)
               .close()
               .header("etag", res.etag.c_str());
  if (res.gzipped) {
    b.header("content-encoding", "gzip");
  }
  return b.build();
}

http::Response Router::process_request(http::Request &r) const {
  http::Response res;
  for (auto const &route : forwarded) {
    if (route.exec_if_match(r, res))
      return res;
  }

  std::string_view endpoint = r.endpoint;
  http::url_params_from_url(endpoint, r.query);
  if (endpoint.size() != 1 && endpoint.ends_with('/'))
    endpoint = endpoint.substr(0, endpoint.size() - 1);
  r.endpoint = endpoint;
  if (r.kind != http::Request::Kind::GET) {
    return http::Response::not_found();
  }
  for (auto const &route : get_routes) {
    if (route.exec_if_match(r, res))
      return res;
  }
  return get_resource(r);
}

http::WSConnection *Router::maybe_process_ws(http::Request &r) const {
  for (auto const &route : ws_routes) {
    if (route.endpoint == r.endpoint) {
      auto ws = route.f(r);
      if (ws)
        ws->origin_endpoint = route.endpoint;
      return ws;
    }
  }
  return nullptr;
}

void Router::clean() {
  get_routes.clear();
  forwarded.clear();
  ws_routes.clear();
}
