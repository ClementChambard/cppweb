#ifndef IG_HTTP_REQUEST_HPP
#define IG_HTTP_REQUEST_HPP

#include <json/value.h>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <defines.hpp>

namespace http {

struct JsonBody : Json::Value {
  bool get_bool(char const *name, bool default_value = false);
  i32 get_int(char const *name, i32 default_value = 0);
  std::string get_string(char const *name, char const *default_value = "");
};

struct Request {
  enum class Kind {
    GET,
    POST,
    PATCH,
    PUT,
    DELETE,
    // TODO:
    ERROR
  } kind;
  std::string endpoint;
  std::string protocol;
  std::unordered_map<std::string, std::string> headers;
  std::string body;
  std::unordered_map<std::string, std::string> params;

  void body_as_params();
  std::optional<JsonBody> body_as_json();

  static Request parse(std::string_view http_request);
  operator std::string();

  std::string first_line();

  std::string header(char const *name, char const *default_value = "");

  std::string string_param(char const *name, char const *default_value = "");
  i32 int_param(char const *name, i32 default_value = 0);
  // TODO: more kinds of params
};

std::string request_kind_to_string(Request::Kind k);


} // namespace http

#endif // !IG_HTTP_REQUEST_HPP
