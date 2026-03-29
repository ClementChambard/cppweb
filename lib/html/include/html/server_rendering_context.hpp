#pragma once

#include <defines.hpp>
#include <cassert>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace html {

struct ServerRenderingContext {
  std::string current_page;
  std::unordered_map<std::string, std::string> page_params;
  std::unordered_map<std::string, std::string> query_params;
  std::string response_type = "text/html";

  void redirect(std::string const &loc) {
    m_redirect = true;
    m_param_str = loc;
  }

  void set_error(std::string const &err) {
    m_error = true;
    m_param_str = err;
  }

  void custom_response(std::string const &response_type, u8 const *data, u64 size);

  std::optional<std::string> get_cookie(std::string const &name) const;

  template <typename T> T &get() {
    assert(user_data != nullptr);
    return *static_cast<T *>(user_data);
  }

  std::vector<u8> get_body(std::string &s);

  void *user_data = nullptr;
  void set_cookies(std::string_view cookies_str);

private:
  std::map<std::string, std::string> m_cookies;
  std::string m_param_str;
  std::vector<u8> m_custom_response;
  bool m_error = false;
  bool m_redirect = false;
  bool m_is_custom_response = false;
};

} // namespace html
