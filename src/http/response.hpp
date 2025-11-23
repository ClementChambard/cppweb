#ifndef IG_HTTP_RESPONSE_HPP
#define IG_HTTP_RESPONSE_HPP

#include <defines.hpp>
#include <string>
#include <unordered_map>

namespace http {

struct Response {
  u32 code;
  std::unordered_map<std::string, std::string> headers;
  std::string body;
  bool has_body;

  static Response ok();
  static Response not_found();
  static Response bad_request();
  static Response unauthorized();

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
    return header("Content-Type", type.c_str())
        .header("Content-Length", std::to_string(body.size()).c_str());
  }
  Builder &emptybody() {
    m_obj.body = "";
    return header("Content-Length", "0");
  }
  Builder &code(u32 code) {
    m_obj.code = code;
    return *this;
  }
  Builder &close() {
    return header("Connection", "close");
  }
  Response build() { return std::move(m_obj); }
  Response m_obj;
};

} // namespace http

#endif // !IG_HTTP_RESPONSE_HPP
