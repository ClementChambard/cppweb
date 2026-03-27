#pragma once

#include <cassert>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
namespace html {

struct ServerRenderingContext {
  std::string current_page;
  std::unordered_map<std::string, std::string> page_params;
  std::unordered_map<std::string, std::string> query_params;

  void redirect(std::string const &loc) {
    m_redirect = true;
    m_param_str = loc;
  }

  void set_error(std::string const &err) {
    m_error = true;
    m_param_str = err;
  }

  std::optional<std::string> get_cookie(std::string const &name) const;

  template <typename T> T &get() {
    assert(user_data != nullptr);
    return *static_cast<T *>(user_data);
  }

  void get_special_html(std::string &s);

  void *user_data = nullptr;
  void set_cookies(std::string_view cookies_str);

private:
  std::map<std::string, std::string> m_cookies;
  std::string m_param_str;
  bool m_error = false;
  bool m_redirect = false;
};

} // namespace html
