#include "config.hpp"
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/writer.h>

void check_required_field(Json::Value const &v, std::string path,
                          std::string prop) {
  if (!v.isMember(prop)) {
    std::string full_name;
    if (path == "")
      full_name = prop;
    else
      full_name = path + "." + prop;
    std::cerr << "Invalid config: missing '" << full_name << "'\n";
    std::exit(EXIT_FAILURE);
  }
}

void check_required_field(Json::Value const &v, std::string prop) {
  check_required_field(v, "", prop);
}

void fill_string_vector(Json::Value const &v, std::string prop,
                        std::vector<std::string> &out) {
  if (!v.isMember(prop))
    return;
  auto arr = v[prop];
  if (!arr.isArray()) {
    // TODO: better error
    std::cerr << "EXPECTED ARRAY\n";
    std::exit(EXIT_FAILURE);
  }
  for (auto &v : arr) {
    out.push_back(v.asString());
  }
}

Config::Config(std::string_view file_name) {
  Json::Value root;
  Json::Reader r;
  std::ifstream f{std::string(file_name)};
  r.parse(f, root);
  if (root.isMember("version"))
    version = root["version"].asString();
  check_required_field(root, "cppweb");
  auto cppweb_o = root["cppweb"];
  check_required_field(cppweb_o, "cppweb", "dir");
  cppweb.dir = cppweb_o["dir"].asString();
  check_required_field(root, "pages");
  auto pages_o = root["pages"];
  check_required_field(pages_o, "pages", "dir");
  pages.dir = pages_o["dir"].asString();
  check_required_field(pages_o, "pages", "build_dir");
  pages.build_dir = pages_o["build_dir"].asString();
  check_required_field(root, "cpp");
  auto cpp_o = root["cpp"];
  check_required_field(cpp_o, "cpp", "src_dir");
  cpp.src_dir = cpp_o["src_dir"].asString();
  check_required_field(cpp_o, "cpp", "build_dir");
  cpp.build_dir = cpp_o["build_dir"].asString();
  check_required_field(cpp_o, "cpp", "lib_name");
  cpp.lib_name = cpp_o["lib_name"].asString();
  if (cpp_o.isMember("use_asan"))
    cpp.use_asan = cpp_o["use_asan"].asBool();
  if (cpp_o.isMember("default_debug"))
    cpp.default_debug = cpp_o["default_debug"].asBool();
  if (cpp_o.isMember("standard"))
    cpp.standard = cpp_o["standard"].asString();
  fill_string_vector(cpp_o, "modules", cpp.modules);
  fill_string_vector(cpp_o, "include_dirs", cpp.include_dirs);
  fill_string_vector(cpp_o, "link_dirs", cpp.link_dirs);
  fill_string_vector(cpp_o, "libraries", cpp.libraries);
  fill_string_vector(cpp_o, "compile_options", cpp.compile_options);
  fill_string_vector(cpp_o, "link_options", cpp.link_options);
  check_required_field(root, "resources");
  auto resources_o = root["resources"];
  check_required_field(resources_o, "resources", "dir");
  resources.dir = resources_o["dir"].asString();
  check_required_field(root, "api");
  auto api_o = root["api"];
  check_required_field(api_o, "api", "root");
  api.root = api_o["root"].asString();
  if (!root.isMember("logger"))
    return;
  auto logger_o = root["logger"];
  logger = logger_t{};
  if (logger_o.isMember("colorize_output"))
    logger->colorize_output = logger_o["colorize_output"].asBool();
  if (logger_o.isMember("log_to_file"))
    logger->log_to_file = logger_o["log_to_file"].asBool();
  if (logger_o.isMember("min_level"))
    logger->min_level = logger_o["min_level"].asString();
  if (logger_o.isMember("log_file"))
    logger->log_file = logger_o["log_file"].asString();
  fill_string_vector(logger_o, "log_extra", logger->log_extra);
  if (!logger_o.isMember("colors"))
    return;
  auto colors_o = logger_o["colors"];
  logger->colors = logger_t::colors_t{};
  if (colors_o.isMember("extra"))
    logger->colors->extra = colors_o["extra"].asString();
  if (colors_o.isMember("extra_kind"))
    logger->colors->extra_kind = colors_o["extra_kind"].asString();
  if (colors_o.isMember("info"))
    logger->colors->info = colors_o["info"].asString();
  if (colors_o.isMember("warn"))
    logger->colors->warn = colors_o["warn"].asString();
  if (colors_o.isMember("error"))
    logger->colors->error = colors_o["error"].asString();
  if (colors_o.isMember("fatal"))
    logger->colors->fatal = colors_o["fatal"].asString();
  if (colors_o.isMember("time"))
    logger->colors->time = colors_o["time"].asString();
}

void write_string_vector(Json::Value &v, std::string const &name,
                         std::vector<std::string> const &values) {
  if (values.size() == 0)
    return;
  Json::Value arr{Json::arrayValue};
  for (auto const &v : values)
    arr.append(v);
  v[name] = arr;
}

void optional_string(Json::Value &v, std::string const &prop,
                     std::optional<std::string> const &val) {
  if (val == std::nullopt)
    return;
  v[prop] = *val;
}

void optional_bool(Json::Value &v, std::string const &prop,
                   std::optional<bool> const &val) {
  if (val == std::nullopt)
    return;
  v[prop] = *val;
}

struct ObjectContext {
  ObjectContext(ObjectContext *last, std::string const &name)
      : name(name), last(last ? &last->cur : nullptr), cur(Json::objectValue),
        latch(0) {}
  std::string name;
  Json::Value *last;
  Json::Value cur;
  int latch;
  void exec_latch() {
    latch++;
    if (last)
      (*last)[name] = cur;
  }
};
#define START_JSON() ObjectContext object_ctx{nullptr, "<root>"};
#define SUB_OBJECT(name)                                                       \
  for (ObjectContext *parent = &object_ctx, object_ctx{parent, name};          \
       object_ctx.latch < 1; object_ctx.exec_latch())
#define CUR_JSON object_ctx.cur
#define SUB_OBJECT_IF(cond, name)                                              \
  if (cond)                                                                    \
  SUB_OBJECT(name)

void Config::write(std::string_view file_name) {
  START_JSON();
  if (version)
    CUR_JSON["version"] = *version;
  SUB_OBJECT("cppweb") { CUR_JSON["dir"] = cppweb.dir; }
  SUB_OBJECT("pages") {
    CUR_JSON["dir"] = pages.dir;
    CUR_JSON["build_dir"] = pages.build_dir;
  }
  SUB_OBJECT("cpp") {
    CUR_JSON["src_dir"] = cpp.src_dir;
    CUR_JSON["build_dir"] = cpp.build_dir;
    CUR_JSON["lib_name"] = cpp.lib_name;
    optional_bool(CUR_JSON, "use_asan", cpp.use_asan);
    optional_bool(CUR_JSON, "default_debug", cpp.default_debug);
    optional_string(CUR_JSON, "standard", cpp.standard);
    write_string_vector(CUR_JSON, "modules", cpp.modules);
    write_string_vector(CUR_JSON, "include_dirs", cpp.include_dirs);
    write_string_vector(CUR_JSON, "link_dirs", cpp.link_dirs);
    write_string_vector(CUR_JSON, "libraries", cpp.libraries);
    write_string_vector(CUR_JSON, "compile_options", cpp.compile_options);
    write_string_vector(CUR_JSON, "link_options", cpp.link_options);
  }
  SUB_OBJECT("resources") { CUR_JSON["dir"] = resources.dir; }
  SUB_OBJECT("api") { CUR_JSON["root"] = api.root; }
  SUB_OBJECT_IF(logger, "logger") {
    optional_bool(CUR_JSON, "colorize_output", logger->colorize_output);
    optional_bool(CUR_JSON, "log_to_file", logger->log_to_file);
    optional_string(CUR_JSON, "min_level", logger->min_level);
    optional_string(CUR_JSON, "log_file", logger->log_file);
    write_string_vector(CUR_JSON, "log_extra", logger->log_extra);
    SUB_OBJECT_IF(logger->colors, "colors") {
      optional_string(CUR_JSON, "extra", logger->colors->extra);
      optional_string(CUR_JSON, "extra_kind", logger->colors->extra_kind);
      optional_string(CUR_JSON, "info", logger->colors->info);
      optional_string(CUR_JSON, "warn", logger->colors->warn);
      optional_string(CUR_JSON, "error", logger->colors->error);
      optional_string(CUR_JSON, "fatal", logger->colors->fatal);
      optional_string(CUR_JSON, "time", logger->colors->time);
    }
  }
  Json::StyledWriter w;
  std::ofstream f{std::string(file_name)};
  f << w.write(CUR_JSON);
}
