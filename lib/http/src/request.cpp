#include <json/reader.h>
#include <optional>
#include <request.hpp>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/logger.hpp>
#include <url_params.hpp>

namespace http {

Request::Kind parse_request_kind(std::string_view k) {
  if (k == "GET")
    return Request::Kind::GET;
  if (k == "POST")
    return Request::Kind::POST;
  if (k == "PATCH")
    return Request::Kind::PATCH;
  if (k == "PUT")
    return Request::Kind::PUT;
  if (k == "DELETE")
    return Request::Kind::DELETE;
  sys::error("KIND IS %*s", k.size(), k.data());
  return Request::Kind::ERROR;
}

void Request::body_as_params() {
  http::UrlParams params;
  http::url_params(body, params);
  this->params.merge(params);
}

bool JsonBody::get_bool(char const *name, bool default_value) {
  if (isMember(name))
    return (*this)[name].asBool();
  return default_value;
}
i32 JsonBody::get_int(char const *name, i32 default_value) {
  if (isMember(name))
    return (*this)[name].asInt();
  return default_value;
}
std::string JsonBody::get_string(char const *name, char const *default_value) {
  if (isMember(name))
    return (*this)[name].asString();
  return default_value;
}

std::optional<JsonBody> Request::body_as_json() {
  static Json::Reader r;

  JsonBody out;
  bool ok = r.parse(body, out);
  if (ok)
    return out;
  return std::nullopt;
}

bool next_space_sep(std::string_view &line, std::string_view &sub) {
  if (line.size() == 0)
    return false;
  u64 space_pos = 0;
  while (space_pos < line.size() && !std::isspace(line[space_pos])) {
    space_pos++;
  }
  sub = line.substr(0, space_pos);
  while (space_pos < line.size() && std::isspace(line[space_pos])) {
    space_pos++;
  }
  if (space_pos == line.size()) {
    line = "";
  } else {
    line = line.substr(space_pos);
  }
  return true;
}

std::string_view next_line(std::string_view &text) {
  u64 line_end_pos = 0;
  while (line_end_pos < text.size() && text[line_end_pos] != '\n' &&
         text[line_end_pos] != '\r') {
    line_end_pos++;
  }
  std::string_view line = text.substr(0, line_end_pos);
  if (line_end_pos == text.size()) {
    text = "";
    return line;
  }
  if (text[line_end_pos] == '\n') {
    if (line_end_pos + 1 < text.size() && text[line_end_pos + 1] == '\r')
      line_end_pos++;
  }
  if (text[line_end_pos] == '\r') {
    if (line_end_pos + 1 < text.size() && text[line_end_pos + 1] == '\n')
      line_end_pos++;
  }
  text = text.substr(line_end_pos + 1);
  return line;
}

void parse_first_line(Request &r, std::string_view first_line) {
  std::string_view next;
  if (!next_space_sep(first_line, next)) {
    r.kind = Request::Kind::ERROR;
    return;
  }
  r.kind = parse_request_kind(next);
  if (!next_space_sep(first_line, next)) {
    r.kind = Request::Kind::ERROR;
    return;
  }
  r.endpoint = next;
  if (!next_space_sep(first_line, next)) {
    r.kind = Request::Kind::ERROR;
    return;
  }
  r.protocol = next;
  if (first_line.size() > 0) {
    r.kind = Request::Kind::ERROR;
    return;
  }
}

void parse_header(Request &r, std::string_view header) {
  u64 colon_pos = 0;
  if (header.size() == 0) {
    r.kind = Request::Kind::ERROR;
    return;
  }
  while (header[colon_pos] != ':') {
    colon_pos++;
    if (colon_pos >= header.size()) {
      r.kind = Request::Kind::ERROR;
      return;
    }
  }
  std::string_view key = header.substr(0, colon_pos);
  std::string_view value = header.substr(colon_pos + 1);
  while (value.size() > 0 && std::isspace(value[0])) {
    value = value.substr(1);
  }
  r.headers[std::string(key)] = value;
}

Request Request::parse(std::string_view http_request) {
  Request r;
  std::string_view first_line = next_line(http_request);
  parse_first_line(r, first_line);
  while (http_request.size() > 0) {
    std::string_view line = next_line(http_request);
    if (line.size() == 0) {
      r.body = http_request;
      break;
    } else {
      parse_header(r, line);
    }
  }
  return r;
}

char const *request_kind_to_string(Request::Kind k) {
  if (k == Request::Kind::GET)
    return "GET";
  if (k == Request::Kind::POST)
    return "POST";
  if (k == Request::Kind::PATCH)
    return "PATCH";
  if (k == Request::Kind::PUT)
    return "PUT";
  if (k == Request::Kind::DELETE)
    return "DELETE";
  return "ERROR";
}

std::string Request::first_line() {
  std::ostringstream oss;
  oss << request_kind_to_string(kind) << " " << endpoint;
  if (query.size() > 0) {
    oss << '?';
    bool put_amp = false;
    for (auto &[k, v] : query) {
      if (put_amp)
        oss << '&';
      oss << k << '=' << v;
    }
  }
  oss << " " << protocol;
  return oss.str();
}

Request::operator std::string() {
  // if (kind == Kind::ERROR) return "INVALID REQUEST";
  std::ostringstream oss;
  oss << first_line() << "\n";
  for (auto [key, value] : headers) {
    oss << key << ": " << value << "\n";
  }
  if (body.size() > 0)
    oss << "\n" << body;
  return oss.str();
}

std::string Request::string_param(char const *name, char const *default_value) {
  if (auto const &it = params.find(name); it != params.end()) {
    return it->second;
  } else {
    return default_value;
  }
}

i32 Request::int_param(char const *name, i32 default_value) {
  if (auto const &it = params.find(name); it != params.end()) {
    return std::stoi(it->second);
  } else {
    return default_value;
  }
}

std::string Request::header(char const *name, char const *default_value) {
  if (auto const &it = headers.find(name); it != headers.end()) {
    return it->second;
  } else {
    return default_value;
  }
}

} // namespace http
