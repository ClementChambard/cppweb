#include <server_rendering_context.hpp>

using namespace cppweb;

std::vector<u8> ServerRenderingContext::get_body(std::string &s) {
  if (m_is_custom_response)
    return m_custom_response;
  static auto pre = "<html><head><title>Error</title></head><body>";
  static auto post = "</body></html>";
  if (m_error) {
    s = pre + ("<h1>Error: " + m_param_str + "</h1>") + post;
  }
  if (m_redirect) {
    s = pre + ("<script>window.location='" + m_param_str + "';</script>") +
        post;
  }
  return {(u8 *)s.data(), (u8 *)s.data() + s.size()};
}

std::optional<std::string>
ServerRenderingContext::get_cookie(std::string const &name) const {
  if (auto it = m_cookies.find(name); it != m_cookies.end()) {
    return it->second;
  }
  return std::nullopt;
}

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

void ServerRenderingContext::set_cookies(std::string_view cursor) {
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

void ServerRenderingContext::custom_response(std::string const &response_type,
                                             u8 const *data, u64 size) {
  this->response_type = response_type;
  m_is_custom_response = true;
  m_custom_response.insert(m_custom_response.end(), data, data + size);
}
