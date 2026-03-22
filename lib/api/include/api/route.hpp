#pragma once

#include "handler.hpp"
#include <http/request.hpp>
#include <map>
#include <string>
#include <vector>

namespace api {

struct PathPart {
  std::string value;
  bool is_param;
};

using RoutePath = std::vector<PathPart>;

bool path_matches(RoutePath const &route, std::vector<std::string> const &check,
                  std::map<std::string, std::string> *o_route_params);

struct Route {
  http::Method method;
  RoutePath path;
  HandlerChain handlers;
};

} // namespace api
