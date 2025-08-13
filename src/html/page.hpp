#ifndef IG_HTML_PAGE_HPP
#define IG_HTML_PAGE_HPP

#include "http/request.hpp"
#include "http/response.hpp"
#include <functional>
#include <string>

namespace html {

struct Page {
  Page(char const *name, std::function<std::string(http::Request &)> build)
      : name(name), build_html(build) {}

  std::string name;
  std::string etag;
  bool needs_rebuild = true;

  static bool default_authorization(http::Request &) { return true; }

  std::function<bool(http::Request &)> check_authorization =
      default_authorization;
  std::function<std::string(http::Request &)> build_html;

  http::Response get_response(http::Request r);
};

inline std::function<http::Response(http::Request)> page(Page &p) {
  return [&p](http::Request r) -> http::Response { return p.get_response(r); };
}

} // namespace html

#endif // !IG_HTML_PAGE_HPP
