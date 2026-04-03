#pragma once

#include <optional>
#include <string>
#include <vector>

struct Config {
  Config() = default;
  Config(std::string_view file_name);
  std::optional<std::string> version; 
  struct {
    std::string dir;
  } cppweb;
  struct {
    std::string dir;
    std::string build_dir;
  } pages;
  struct {
    std::string src_dir;
    std::string build_dir;
    std::string lib_name;
    std::optional<bool> use_asan;
    std::optional<bool> default_debug;
    std::optional<std::string> standard;
    std::vector<std::string> modules;
    std::vector<std::string> include_dirs;
    std::vector<std::string> link_dirs;
    std::vector<std::string> libraries;
    std::vector<std::string> compile_options;
    std::vector<std::string> link_options;
  } cpp;
  struct {
    std::string dir;
  } resources;
  struct {
    std::string root;
  } api;
  struct logger_t{
    std::optional<bool> colorize_output;
    std::optional<bool> log_to_file;
    std::optional<std::string> min_level;
    std::optional<std::string> log_file;
    std::vector<std::string> log_extra;
    struct colors_t {
      std::optional<std::string> extra;
      std::optional<std::string> extra_kind;
      std::optional<std::string> info;
      std::optional<std::string> warn;
      std::optional<std::string> error;
      std::optional<std::string> fatal;
      std::optional<std::string> time;
    };
    std::optional<colors_t> colors;
  };
  std::optional<logger_t> logger;
  void write(std::string_view file_name);

  bool dev = false;
  std::string cpp_bin() const {
    return cpp.build_dir + "/lib" + cpp.lib_name + ".so";
  }
};

extern Config CONFIG;
