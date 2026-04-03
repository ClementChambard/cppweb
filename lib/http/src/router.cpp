#include <cassert>
#include <request.hpp>
#include <resource.hpp>
#include <response.hpp>
#include <router.hpp>
#include <string_view>
#include <sys/read_file.hpp>
#include <unordered_map>
#include <url_params.hpp>

namespace http {

Route::Route(std::string_view endpoint, EndpointFunc f)
    : route_decl(endpoint), f(f) {
  assert(false);
}

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

bool check_match(Request &r, Route const &route, UrlParams &route_params) {
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
                                         url_decode(request_route[i])));
    } else if (route.route_parts[i].is_catchall) {
      std::string const &param_name = route.route_parts[i].value;
      std::string out;
      while (i < request_route.size()) {
        out += '/';
        out += url_decode(request_route[i]);
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

bool Route::exec_if_match(Request &r, Response &res) const {
  UrlParams route_params;
  if (!check_match(r, *this, route_params))
    return false;

  for (auto [k, v] : route_params) {
    r.params[k] = v;
  }

  res = this->f(r);
  return true;
}

bool ForwardedUrl::exec_if_match(Request &r, Response &res) const {
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

Router &Router::route(char const *endpoint, Request::Kind k, EndpointFunc fn) {
  if (k == Request::Kind::POST)
    post_routes.push_back({endpoint, fn});
  if (k == Request::Kind::GET)
    get_routes.push_back({endpoint, fn});
  if (k == Request::Kind::PATCH)
    patch_routes.push_back({endpoint, fn});
  if (k == Request::Kind::PUT)
    put_routes.push_back({endpoint, fn});
  if (k == Request::Kind::DELETE)
    delete_routes.push_back({endpoint, fn});
  return *this;
}

Response get_resource(Request r) {
  Resource const &res = Resource::get(r.endpoint);
  if (!res.info.exists) {
    return Response::not_found();
  }
  if (auto header_it = r.headers.find("If-None-Match");
      header_it != r.headers.end()) {
    if (res.etag == header_it->second) {
      return Response::Builder()
          .code(304)
          .header("etag", res.etag.c_str())
          .close()
          .build();
    }
  }
  std::string file_content = res.get_res_contents();
  auto b = Response::Builder()
               .code(200)
               .body(res.resource_type, file_content)
               .close()
               .header("etag", res.etag.c_str());
  if (res.gzipped) {
    b.header("content-encoding", "gzip");
  }
  return b.build();
}

Response Router::process_request(Request &r) const {
  Response res;
  for (auto const &route : forwarded) {
    if (route.exec_if_match(r, res))
      return res;
  }

  std::string_view endpoint = r.endpoint;
  url_params_from_url(endpoint, r.query);
  if (endpoint.size() != 1 && endpoint.ends_with('/'))
    endpoint = endpoint.substr(0, endpoint.size() - 1);
  r.endpoint = endpoint;
  if (r.kind == Request::Kind::POST) {
    for (auto const &route : post_routes) {
      if (route.exec_if_match(r, res))
        return res;
    }
  }
  if (r.kind == Request::Kind::PATCH) {
    for (auto const &route : patch_routes) {
      if (route.exec_if_match(r, res))
        return res;
    }
  }
  if (r.kind == Request::Kind::PUT) {
    for (auto const &route : put_routes) {
      if (route.exec_if_match(r, res))
        return res;
    }
  }
  if (r.kind == Request::Kind::DELETE) {
    for (auto const &route : delete_routes) {
      if (route.exec_if_match(r, res))
        return res;
    }
  }
  if (r.kind == Request::Kind::GET) {
    for (auto const &route : get_routes) {
      if (route.exec_if_match(r, res))
        return res;
    }
    return get_resource(r);
  }
  return Response::not_found();
}

void Router::clean() {
  get_routes.clear();
  post_routes.clear();
  patch_routes.clear();
  put_routes.clear();
  delete_routes.clear();
  forwarded.clear();
}

} // namespace http
