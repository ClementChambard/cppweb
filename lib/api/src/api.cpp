#include "context.hpp"
#include "http/response.hpp"
#include "http/url_params.hpp"
#include "route.hpp"
#include <api.hpp>
#include <cassert>
#include <iostream>

namespace api {

void get_body(Context &ctx, http::Request const &req) {
  // TODO: check content type
  static Json::Reader r;

  Json::Value out;
  bool ok = r.parse(req.body, out);
  if (ok) {
    ctx.request_body = out;
  }
}

std::vector<std::string> path_get(std::string route,
                                  http::UrlParams *query_params) {
  std::vector<std::string> out;
  assert(route.starts_with('/'));

  size_t pos = 1;
  while (pos < route.size()) {
    size_t end_pos = route.find('/', pos);
    auto sub = route.substr(pos, end_pos - pos);
    if (sub.size() != 0) {
      out.push_back(sub);
    }
    if (end_pos == std::string::npos)
      break;
    pos = end_pos + 1;
  }

  if (out.size() == 0)
    return out;
  std::string query = "";
  auto &last = out.back();

  if ((pos = last.find('?')) != std::string::npos) {
    query = last.substr(pos + 1);
    last.resize(pos);
  }

  if (last.size() == 0 && out.size() == 1) out.clear();

  if (query_params && query != "") {
    http::url_params(query, *query_params);
  }

  return out;
}

http::Response Api::dispatch_request(std::string route,
                                     http::Request const &req) {
  Context ctx;

  get_body(ctx, req);

  bool found = false;

  std::vector<std::string> route_path = path_get(route, &ctx.query_params);

  for (auto const &r : routes) {
    if (req.kind != r.method) continue;
    if (path_matches(r.path, route_path, &ctx.route_params)) {
      execute_handler_chain(r.handlers, ctx);
      found = true;
      break;
    }
  }

  if (!found) {
    ctx.res.code(404);
  }

  return ctx.res.close().build();
}

std::function<http::Response(http::Request)> Api::get_func() {
  return [this](http::Request r) -> http::Response {
    std::string route = r.endpoint;
    assert(route.starts_with(base_url));
    route = route.substr(base_url.size());
    if (!route.starts_with('/')) route.insert(0, "/");
    return dispatch_request(route, r);
  };
}

} // namespace api
