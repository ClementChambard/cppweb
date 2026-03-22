#include <route.hpp>

namespace api {
bool path_matches(RoutePath const &route, std::vector<std::string> const &check,
                  std::map<std::string, std::string> *o_route_params) {
  if (route.size() != check.size()) return false;

  std::map<std::string, std::string> params;

  for (u32 i = 0; i < route.size(); i++) {
    if (route[i].is_param) {
      params.insert_or_assign(route[i].value, check[i]);
    } else {
      if (route[i].value != check[i]) return false;
    }
  }

  if (o_route_params) {
    o_route_params->insert(params.begin(), params.end());
  }
  return true;
}

} // namespace api
