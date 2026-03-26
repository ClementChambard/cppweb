#include <response.hpp>
#include <sstream>

namespace http {

std::string code_string(u32 code) {
  std::string name;
  if (code == 200)
    name = " OK";
  if (code == 204)
    name = " No Content";
  if (code == 205)
    name = " Reset Content";
  if (code == 302)
    name = " Not Modified";
  if (code == 303)
    name = " See Other";
  if (code == 400)
    name = " Bad Request";
  if (code == 401)
    name = " Unauthorized";
  if (code == 404)
    name = " Not Found";
  if (code == 405)
    name = " Method Not Allowed";

  return std::to_string(code) + name;
}

std::string Response::first_line() {
  std::ostringstream oss;
  oss << "HTTP/1.1 " << code_string(code);
  return oss.str();
}

Response::operator std::string() {
  std::ostringstream oss;
  oss << first_line() << "\r\n";
  for (auto [key, value] : headers) {
    oss << key << ": " << value << "\r\n";
  }
  oss << "\r\n" << body;
  return oss.str();
}

Response error_response(u32 code) {
  return Response::Builder()
      .code(code)
      .body("text/*; charset=UTF-8",
            std::to_string(code) + " " + code_string(code))
      .close()
      .build();
}

Response Response::ok() {
  return Response::Builder().code(204).close().build();
}

Response Response::created() {
  return Response::Builder().code(201).close().build();
}

Response Response::not_found() { return error_response(404); }

Response Response::bad_request() { return error_response(400); }

Response Response::unauthorized() { return error_response(401); }

std::string SetCookie::get() const {
  auto out = name + "=" + value;

  if (path != std::nullopt) {
    out += "; Path=" + *path;
  }

  return out;
}

} // namespace http
