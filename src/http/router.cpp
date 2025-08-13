#include "router.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "http/url_params.hpp"
#include "sys/read_file.hpp"
#include <string_view>
#include <unordered_map>

namespace http {

void split_route_placeholders(std::string_view route,
                              std::vector<std::string_view> &parts,
                              std::vector<std::string_view> &placeholders) {
  while (route.size() > 0) {
    u64 pos = 0;
    while (pos < route.size() && route[pos] != '<') {
      pos++;
    }
    if (pos == route.size()) {
      parts.push_back(route);
      return;
    }
    parts.push_back(route.substr(0, pos));
    route = route.substr(pos + 1);
    pos = 0;
    // TODO: error checking
    while (route[pos] != '>') {
      pos += 1;
    }
    if (pos == route.size()) {
      placeholders.push_back(route);
      return;
    }
    placeholders.push_back(route.substr(0, pos));
    route = route.substr(pos + 1);
  }
}

Route::Route(std::string_view endpoint, EndpointFunc f)
    : route_decl(endpoint), f(f) {
  split_route_placeholders(endpoint, route_parts, placeholder_names);
}

bool check_match(Request &r, Route const &route, UrlParams &params) {
  if (route.placeholder_names.size() == 0 && route.route_parts.size() == 1) {
    return r.endpoint == route.route_decl;
  }
  std::string_view endpoint = r.endpoint;
  if (!endpoint.starts_with(route.route_parts[0])) {
    return false;
  }
  endpoint = endpoint.substr(route.route_parts[0].size());
  u64 i = 1;
  while (true) {
    // decode potential placeholder
    if (i > route.placeholder_names.size()) {
      return endpoint.empty();
    }
    if (i == route.route_parts.size()) {
      params[std::string(route.placeholder_names[i - 1])] =
          decode_param(endpoint);
      return true;
    }
    auto pos = endpoint.find(route.route_parts[i]);
    if (pos == std::string_view::npos) {
      return false;
    }
    params[std::string(route.placeholder_names[i - 1])] =
        decode_param(endpoint.substr(0, pos));
    endpoint = endpoint.substr(pos + route.route_parts[i].size());
    i++;
  }
}

bool Route::exec_if_match(Request &r, Response &res) const {
  UrlParams decoded_params;
  if (!check_match(r, *this, decoded_params))
    return false;
  for (auto [k, v] : decoded_params) {
    r.params[k] = v;
  }
  res = this->f(r);
  return true;
}

Router &Router::route(char const *endpoint, Request::Kind k, EndpointFunc fn) {
  if (k == Request::Kind::POST)
    post_routes.push_back({endpoint, fn});
  if (k == Request::Kind::GET)
    get_routes.push_back({endpoint, fn});
  if (k == Request::Kind::PATCH)
    patch_routes.push_back({endpoint, fn});
  return *this;
}

std::string resource_type(std::string_view filename) {
  if (filename.ends_with(".css"))
    return "text/css; charset=UTF-8";
  if (filename.ends_with(".ico"))
    return "image/x-icon";
  return "*/*";
}

struct Resource {
  std::string file_name;
  std::string resource_type;
  bool gzipped;
  std::string etag;
  sys::FInfo info;
};

std::unordered_map<std::string, Resource> resource_cache;

void gzipped(Resource &res) {
  auto gzip_filename = res.file_name + ".gz";
  auto info = sys::file_info(gzip_filename.c_str());
  if (!info.exists) return;
  res.file_name = std::move(gzip_filename);
  res.gzipped = true;
  res.info = info;
}

Response get_resource(Request r) {
  if (auto it = resource_cache.find(r.endpoint); it != resource_cache.end()) {
    auto &res = it->second;
    auto info = sys::file_info(res.file_name.c_str());
    if (!info.exists) {
      return Response::not_found();
    }
    res.info = info;
    res.etag = "\"" + std::to_string(info.last_modified) + "\"";
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
  } else {
    Resource res;
    res.file_name = "public" + r.endpoint;
    res.resource_type = resource_type(res.file_name);
    res.gzipped = res.file_name.ends_with(".gz");
    res.info = sys::file_info(res.file_name.c_str());
    if (!res.gzipped && !res.info.exists) {
      gzipped(res);
    }
    res.etag = "\"" + std::to_string(res.info.last_modified) + "\"";
    resource_cache[r.endpoint] = res;
    if (!res.info.exists) {
      return Response::not_found();
    }
  }
  Resource &res = resource_cache[r.endpoint];
  std::string file_content = sys::read_text_file(res.file_name.c_str());
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
  std::string_view endpoint = r.endpoint;
  url_params_from_url(endpoint, r.params);
  if (endpoint.size() != 1 && endpoint.ends_with('/')) endpoint = endpoint.substr(0, endpoint.size() - 1);
  r.endpoint = endpoint;
  Response res;
  if (r.kind == Request::Kind::POST) {
    for (auto const &route : post_routes) {
      if (route.exec_if_match(r, res))
        return res;
    }
    return Response::not_found();
  }
  if (r.kind == Request::Kind::PATCH) {
    for (auto const &route : patch_routes) {
      if (route.exec_if_match(r, res))
        return res;
    }
    return Response::Builder().code(405).close().build();
  }
  if (r.kind == Request::Kind::GET) {
    for (auto const &route : get_routes) {
      if (route.exec_if_match(r, res))
        return res;
    }
    return get_resource(r);
  }
  return Response::bad_request();
}

} // namespace http
