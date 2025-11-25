#include "url_params.hpp"
#include "sys/logger.hpp"
#include <defines.hpp>
#include <string_view>

namespace http {

char get_pcchar(char c1, char c2) {
  auto digit = [](char c) -> int {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    sys::error("   * INVALID CHARACTER IN %%XX SEQUENCE");
    return 0;
  };
  return static_cast<char>(0x10 * digit(c1) + digit(c2));
}

std::string decode_param(std::string_view param) {
  auto p = std::string(param);
  u64 prev_loc = 0;
  u64 loc = 0;
  while ((loc = p.find('+', prev_loc)) != std::string::npos) {
    p[loc] = ' ';
    prev_loc = loc + 1;
  }
  loc = prev_loc = 0;
  while ((loc = p.find('%', prev_loc)) != std::string::npos) {
    auto c = get_pcchar(p[loc + 1], p[loc + 2]);
    p.replace(loc, 3, "X");
    p[loc] = c; // TODO: is that good ?
    prev_loc = loc + 1;
  }
  return p;
}

static void url_param(std::string_view par, UrlParams &params) {
  if (par.empty()) {
    sys::error("   * EMPTY URL PARAMETER");
    return;
  }
  auto equals_loc = par.find('=');
  if (equals_loc == std::string_view::npos) {
    sys::error("   * MISSING '=' IN URL PARAMETER: %*s", par.size(), par.data());
    return;
  }
  auto name = decode_param(par.substr(0, equals_loc));
  if (equals_loc + 1 == par.size()) {
    params[name] = "";
  } else {
    params[name] = decode_param(par.substr(equals_loc + 1));
  }
}

void url_params(std::string_view str, UrlParams &params) {
  if (str.empty()) {
    sys::warn("   * PARSING EMPTY URL PARAMETER");
    return;
  }
  u64 loc = 0;
  u64 prev_loc = loc;
  while ((loc = str.find('&', prev_loc)) != std::string::npos) {
    u64 len = loc - prev_loc;
    url_param(str.substr(prev_loc, len), params);
    prev_loc = loc + 1;
  }
  url_param(str.substr(prev_loc), params);
}

void decode_cookies(std::string_view str, UrlParams &params) {
  u64 loc = 0;
  u64 prev_loc = loc;
  while ((loc = str.find("; ", prev_loc)) != std::string::npos) {
    u64 len = loc - prev_loc;
    url_param(str.substr(prev_loc, len), params);
    prev_loc = loc + 2;
  }
  url_param(str.substr(prev_loc), params);
}

void url_params_from_url(std::string_view &url, UrlParams &params) {
  auto question_loc = url.find('?');
  if (question_loc == std::string_view::npos) {
    return;
  }
  std::string_view params_str = url.substr(question_loc + 1);
  url = url.substr(0, question_loc);
  url_params(params_str, params);
}

} // namespace http
