#pragma once

#include <defines.hpp>
#include <json/json.h>
#include <string>
#include <unordered_map>

namespace http {

struct SetCookie {
  std::string name;
  std::string value;
  std::optional<std::string> path{};
  // TODO:

  std::string get() const;
};

struct Response {
  u32 code;
  std::unordered_map<std::string, std::string> headers;
  std::vector<u8> body;

  static Response ok();
  static Response created();
  static Response not_found();
  static Response bad_request();
  static Response unauthorized();

  std::string first_line();
  operator std::vector<u8>();

  struct Builder;
};

struct Response::Builder {
  Builder &header(char const *key, char const *value) {
    m_obj.headers[key] = value;
    return *this;
  }
  Builder &body(std::string const &type, std::string const &body) {
    m_obj.body.insert(m_obj.body.begin(), (u8 *)body.data(),
                      (u8 *)body.data() + body.size());
    m_has_body = true;
    return header("Content-Type", type.c_str())
        .header("Content-Length", std::to_string(body.size()).c_str());
  }
  Builder &body(std::string const &type, std::vector<u8> const &body) {
    m_obj.body.insert(m_obj.body.begin(), (u8 *)body.data(),
                      (u8 *)body.data() + body.size());
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

  Builder &set_cookie(SetCookie cookie) {
    // TODO: support multiple cookies
    return header("Set-Cookie", cookie.get().c_str());
  }
  Builder &set_cookie(std::string const &name, std::string const &value) {
    return set_cookie({name, value});
  }

  Response build(bool nobody = false) {
    if (!m_has_body && !nobody)
      header("Content-Length", "0");
    return std::move(m_obj);
  }
  Response m_obj;
  bool m_has_body = false;
};

} // namespace http
