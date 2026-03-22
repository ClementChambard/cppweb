#pragma once

#include <defines.hpp>
#include <json/json.h>
#include <string>
#include <unordered_map>

namespace http {

struct Response {
  u32 code;
  std::unordered_map<std::string, std::string> headers;
  std::string body;

  static Response ok();
  static Response created();
  static Response not_found();
  static Response bad_request();
  static Response unauthorized();

  std::string first_line();
  operator std::string();

  struct Builder;
};

struct Response::Builder {
  Builder &header(char const *key, char const *value) {
    m_obj.headers[key] = value;
    return *this;
  }
  Builder &body(std::string const &type, std::string const &body) {
    m_obj.body = body;
    m_has_body = true;
    return header("Content-Type", type.c_str())
        .header("Content-Length", std::to_string(body.size()).c_str());
  }
  Builder &json(Json::Value const &v) {
    static Json::FastWriter writer{};
    return body("text/json", writer.write(v));
  }
  Builder &code(u32 code) {
    m_obj.code = code;
    return *this;
  }
  Builder &close() { return header("Connection", "close"); }
  Response build(bool nobody = false) {
    if (!m_has_body && !nobody)
      header("Content-Length", "0");
    return std::move(m_obj);
  }
  Response m_obj;
  bool m_has_body = false;
};

} // namespace http
