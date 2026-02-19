#include "http/response.hpp"
#include "http/url_params.hpp"
#include "routes.hpp"

http::Response api::check_admin(http::Request r) {
  r.body_as_params();
  auto mdp = r.string_param("password");
  if (mdp == "") {
    return http::Response::bad_request();
  }
  if (mdp == "MamanQueJ'Aime") {
    return http::Response::Builder()
        .code(303)
        .header("location", "/admin/sondages")
        .header("Set-cookie", "sessionId=abc123; Path=/")
        .close()
        .build();
  } else {
    return http::Response::unauthorized();
  }
}

bool is_authentified(http::Request r) {
  http::UrlParams cookies;
  http::decode_cookies(r.header("Cookie"), cookies);
  auto it = cookies.find("sessionId");
  return it != cookies.end() && it->second == "abc123";
}
