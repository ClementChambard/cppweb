#include "page.hpp"
#include "http/request.hpp"
#include "sys/read_file.hpp"
#include "sys/subprocess.hpp"
#include <chrono>

namespace html {

static std::string get_page_filename(Page &p) {
  return "html/tmp/" + p.name + ".html.gz";
}

static bool check_page_created(Page &p) {
  return sys::file_info(get_page_filename(p).c_str()).exists;
}

void rebuild(Page &p, http::Request &r) {
  std::string html = p.build_html(r);

  std::string filename = std::string("html/tmp/") + p.name + ".html";
  sys::write_text_file(filename.c_str(), html);

  char const *args[] = {"gzip", "-f", filename.c_str()};
  sys::subprocess_run(args);

  p.etag =
      '"' +
      std::to_string(std::chrono::utc_clock::now().time_since_epoch().count()) +
      '"';
  p.needs_rebuild = false;
}

http::Response Page::get_response(http::Request r) {
  if (!check_authorization(r))
    return http::Response::unauthorized();
  
  if (needs_rebuild || !check_page_created(*this)) {
    rebuild(*this, r);
  } else if (auto header_it = r.headers.find("If-None-Match");
             header_it != r.headers.end()) {
    if (etag == header_it->second) {
      return http::Response::Builder()
          .code(304)
          .header("etag", etag.c_str())
          .close()
          .build();
    }
  }
  return http::Response::Builder()
      .code(200)
      .body("text/html; charset=utf-8", sys::read_text_file(get_page_filename(*this).c_str()))
      .header("content-encoding", "gzip")
      .header("etag", etag.c_str())
      .close()
      .build();
}

} // namespace html
