#ifndef IG_HTML_PAGE_HPP
#define IG_HTML_PAGE_HPP

#include "http/request.hpp"
#include "http/response.hpp"
#include <functional>
#include <string>

namespace html {

struct Page {
  Page(char const *route);

  std::string name;
  std::string etag = "";
  std::function<bool(http::Request &)> check_authorization = [](http::Request&) -> bool { return true; };
  std::function<std::string(http::Request &)> build_html;

  http::Response get_response(http::Request r);
};

inline std::function<http::Response(http::Request)> page(Page const &p) {
  struct Wrapper {
    mutable Page p;
  } w{p};
  return [w](http::Request r) -> http::Response { return w.p.get_response(r); };
}


} // namespace html

#endif // !IG_HTML_PAGE_HPP
