#include "page.hpp"
#include "html/code_builder.hpp"
#include "http/request.hpp"
#include "sys/read_file.hpp"
#include "sys/subprocess.hpp"
#include <chrono>
#include <filesystem>

namespace html {

static std::string get_page_filename(Page &p) {
  std::string name = p.name;
  for (auto &c : name) if (c == '/') c = '_';
  return "html/tmp/" + name + ".html";
}

static bool check_page_created(Page &p) {
  return sys::file_info((get_page_filename(p) + ".gz").c_str()).exists;
}

void rebuild(Page &p, http::Request &r) {
  std::string html = p.build_html(r);

  std::string filename = get_page_filename(p);
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
      .body("text/html; charset=utf-8", sys::read_text_file((get_page_filename(*this) + ".gz").c_str()))
      .header("content-encoding", "gzip")
      .header("etag", etag.c_str())
      .close()
      .build();
}

void maybe_add_page_file(std::string const &name, std::vector<std::string> & out) {
  auto file_name = "html/root" + name;
  if (!std::filesystem::exists(std::filesystem::path(file_name))) return;
  file_name = file_name.substr(5);
  size_t pos = 0;
  while ((pos = file_name.find("/", pos)) != std::string::npos) {
    file_name.replace(pos, 1, "::");
  }
  out.push_back(file_name.substr(0, file_name.size() - 5));
}


Page Page::from_route(const char *route) {
  std::string rte = route;
  std::vector<std::string> segments;
  size_t idx = 0;
  if (rte == "/") {
    maybe_add_page_file("/layout.html", segments);
    maybe_add_page_file("/page.html", segments);
  } else {
    while (true) {
      idx = rte.find('/', idx);
      if (idx == std::string::npos) {
        maybe_add_page_file(rte + "/layout.html", segments);
        maybe_add_page_file(rte + "/page.html", segments);
        break;
      } else {
        maybe_add_page_file(rte.substr(0, idx) + "/layout.html", segments);
      }
      idx += 1;
    }
  }
  return Page(route, [segments](http::Request &) -> std::string {
    std::string out = "";
    for (auto i = segments.size(); i > 0; i--) {
      CodeBuilder b{segments[i-1].c_str()};
      b.placeholder("children", out);
      out = b.build();
    }
    return out;
  });
}



} // namespace html
