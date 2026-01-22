#include "resource.hpp"
#include "sys/read_file.hpp"
#include <optional>
#include <unordered_map>
#include <defines.hpp>

namespace http {

static void setup_resource_type(Resource &res) {
  if (res.file_name.ends_with(".css"))
    res.resource_type = "text/css; charset=UTF-8";
  if (res.file_name.ends_with(".ico"))
    res.resource_type = "image/x-icon";
  res.resource_type = "*/*";
  res.gzipped = res.file_name.ends_with(".gz");
}

static void setup_gzipped(Resource &res) {
  auto gzip_filename = res.file_name + ".gz";
  auto info = sys::file_info(gzip_filename.c_str());
  if (!info.exists) {
    return;
  }
  res.file_name = std::move(gzip_filename);
  res.gzipped = true;
  res.info = info;
}

Resource::Resource(std::string const &filename) : file_name(filename) {
  setup_resource_type(*this);

  info = sys::file_info(file_name.c_str());
  if (!gzipped && !info.exists) {
    setup_gzipped(*this);
  }
  etag = "\"" + std::to_string(info.last_modified) + "\"";
}

static std::unordered_map<std::string, Resource> g_RESOURCE_CACHE;

static void cache_results_of_res(Resource &res, std::optional<sys::FInfo> old_info = std::nullopt) {
  res.cache_contents = true;

  // check if resource exists
  if (!res.info.exists) {
    res.cached_contents = "";
    return;
  }

  // check if resource changed
  if (old_info != std::nullopt) {
    sys::FInfo &info = *old_info;
    if (info.exists && res.info.last_modified == info.last_modified) {
      return;
    }
  }

  // do the caching
  res.cached_contents = sys::read_text_file(res.file_name.c_str());
}

Resource const &Resource::get(std::string const &name, bool cache_results) {
  std::string file_name;
  if (name.starts_with('/')) {
    file_name = "public" + name;
  } else if (name.starts_with("html::")) {
    file_name = name + ".html";
    u64 pos = 0;
    while ((pos = file_name.find("::", pos)) != std::string::npos) {
      file_name.replace(pos, 2, "/");
    }
  } else if (name.starts_with("page::")) {
    file_name = "html/tmp/" + name.substr(6) + ".html.gz";
  }
  if (auto const &it = g_RESOURCE_CACHE.find(name); it != g_RESOURCE_CACHE.end()) {
    auto &res = it->second;
    auto old_info = res.info;
    res.info = sys::file_info(res.file_name.c_str());
    if (res.info.exists) {
      res.etag = "\"" + std::to_string(res.info.last_modified) + "\"";
    }
    if (cache_results || res.cache_contents) cache_results_of_res(res, old_info);
    return res;
  } else {
    Resource res(file_name);
    if (cache_results || res.cache_contents) cache_results_of_res(res);
    g_RESOURCE_CACHE[name] = res;
    return g_RESOURCE_CACHE[name];
  }
}

std::string Resource::get_res_contents() const {
  if (cache_contents) return cached_contents;
  return sys::read_text_file(file_name.c_str());
}

} // namespace http
