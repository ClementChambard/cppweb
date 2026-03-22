#pragma once

#include <any>
#include <http/response.hpp>
#include <http/url_params.hpp>
#include <json/value.h>
#include <map>
#include <string>

namespace api {

struct Context {
  http::UrlParams query_params;
  std::map<std::string, std::string> route_params;
  Json::Value request_body;
  http::Response::Builder res;

  template <typename T> void set_data(std::string const &name, T const &value) {
    m_data.insert_or_assign(name, value);
  }
  template <typename T> T &get_data(std::string const &name) {
    return std::any_cast<T &>(m_data[name]);
  }

  bool ok(u32 code = 200) { res.code(code); return true; }
  bool error(u32 code) { res.code(code); return false; }

private:
  std::map<std::string, std::any> m_data;
};

} // namespace api
