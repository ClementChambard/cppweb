#pragma once

#include <defines.hpp>
#include <json/value.h>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace http {

struct JsonBody : Json::Value {
  bool get_bool(char const *name, bool default_value = false);
  i32 get_int(char const *name, i32 default_value = 0);
  std::string get_string(char const *name, char const *default_value = "");
};

enum class Method {
    GET,
    POST,
    PATCH,
    PUT,
    DELETE,
    // TODO:
    ERROR
};

struct Request {
  using Kind = Method;
  Kind kind;
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

char const * request_kind_to_string(Request::Kind k);

} // namespace http
