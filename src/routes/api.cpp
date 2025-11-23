#include "../db.hpp"
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
        .emptybody()
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

http::Response api::sondages_id(http::Request r) {
  r.body_as_params();

  if (!is_authentified(r))
    return http::Response::unauthorized();

  i32 idx = r.int_param("id");
  SondagesDb::lock();
  SondagesDb::get().items[idx].active =
      r.int_param("active", SondagesDb::get().items[idx].active);
  SondagesDb::get().items[idx].name =
      r.string_param("name", SondagesDb::get().items[idx].name.c_str());
  SondagesDb::get().items[idx].button_text = r.string_param(
      "btn_text", SondagesDb::get().items[idx].button_text.c_str());
  SondagesDb::get().items[idx].desc =
      r.string_param("desc", SondagesDb::get().items[idx].desc.c_str());
  SondagesDb::get().items[idx].route =
      r.string_param("route", SondagesDb::get().items[idx].route.c_str());
  SondagesDb::get().write();
  SondagesDb::unlock();
  return http::Response::Builder()
      .code(303)
      .header("location", r.header("Referer").c_str())
      .emptybody()
      .close()
      .build();
}
