#include <context.hpp>

static std::string_view trim(std::string_view s) {
  while (s.size() > 0 && std::isspace(s[0])) {
    if (s.size() <= 1)
      return s;
    s = s.substr(1);
  }
  while (s.size() > 0 && std::isspace(s[s.size() - 1])) {
    s = s.substr(0, s.size() - 1);
  }
  return s;
}

std::optional<std::string> api::Context::get_cookie(std::string const &name) {
  if (auto it = m_cookies.find(name); it != m_cookies.end()) {
    return it->second;
  }
  return std::nullopt;
}

void api::Context::set_req_data(http::Request const &req) {
  // TODO: check content type
  static Json::Reader r;

  Json::Value out;
  bool ok = r.parse(req.body, out);
  if (ok) {
    request_body = out;
  }

  if (auto it = req.headers.find("Cookie"); it != req.headers.end()) {
    auto cookies_header = it->second;
    std::string_view cursor = cookies_header;
    while (true) {
      auto pos = cursor.find(';');
      std::string_view sub_cookies = cursor.substr(0, pos);
      auto eq = sub_cookies.find('=');
      auto cookie_name = trim(sub_cookies.substr(0, eq));
      auto cookie_value = trim(sub_cookies.substr(eq + 1));
      m_cookies.insert(
          std::make_pair(std::string(cookie_name), std::string(cookie_value)));

      if (pos == std::string_view::npos)
        break;
      cursor = cursor.substr(pos + 1);
    }
  }
}
