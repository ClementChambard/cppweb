#include "response.hpp"
#include "html/code_builder.hpp"
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

Response::operator std::string() {
  std::ostringstream oss;
  oss << "HTTP/1.1 " << code_string(code) << "\r\n";
  for (auto [key, value] : headers) {
    oss << key << ": " << value << "\r\n";
  }
  oss << "\r\n" << body;
  return oss.str();
}

Response error_response(u32 code) {
  auto html = html::CodeBuilder("page::error")
                  .placeholder("code", std::to_string(code).c_str())
                  .placeholder("error", code_string(code).c_str())
                  .build();
  return Response::Builder()
      .code(code)
      .body("text/html; charset=UTF-8", html)
      .close()
      .build();
}

Response Response::ok() {
  return Response::Builder().code(204).close().build();
}

Response Response::not_found() { return error_response(404); }

Response Response::bad_request() { return error_response(400); }

Response Response::unauthorized() { return error_response(401); }

} // namespace http
