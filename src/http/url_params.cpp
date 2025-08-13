#include "url_params.hpp"
#include "sys/logger.hpp"
#include <defines.hpp>
#include <string_view>

namespace http {

char const *get_char(char c1, char c2) {
  if (c1 == '2' && c2 == '7') return "'";
  return "%";
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
    p.replace(loc, 3, get_char(p[loc + 1], p[loc + 2]));
    prev_loc = loc + 1;
  }
  return p;
}

static void url_param(std::string_view par, UrlParams &params) {
  auto equals_loc = par.find('=');
  if (equals_loc == std::string_view::npos) {
    sys::error("INVALID URL PARAMETER: %*s", par.size(), par.data());
    return;
  }
  auto name = par.substr(0, equals_loc);
  if (equals_loc + 1 == par.size()) {
    params[std::string(name)] = "";
  } else {
    params[std::string(name)] = decode_param(par.substr(equals_loc + 1));
  }
}

void url_params(std::string_view str, UrlParams &params) {
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
