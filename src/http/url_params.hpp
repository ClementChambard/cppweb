#ifndef IG_HTTP_URL_PARAMS_HPP
#define IG_HTTP_URL_PARAMS_HPP

#include <string>
#include <string_view>
#include <unordered_map>

namespace http {

using UrlParams = std::unordered_map<std::string, std::string>;

std::string decode_param(std::string_view str);

void url_params(std::string_view str, UrlParams &params);

void url_params_from_url(std::string_view &url, UrlParams &params);

void decode_cookies(std::string_view str, UrlParams &cookies);

} // namespace http

#endif // !IG_HTTP_URL_PARAMS_HPP
