#include "config.hpp"
#include <fstream>
#include <json/json.h>

Config::Config(std::string const &file_name) {
  Json::Value root;
  Json::Reader r;
  std::ifstream f(file_name);
  r.parse(f, root);
  if (root.isMember("cppweb_dir"))
    cppweb_dir = root["cppweb_dir"].asString();
  if (root.isMember("pages")) {
    auto v = root["pages"];
    if (v.isMember("dir"))
      pages_dir = v["dir"].asString();
    if (v.isMember("build_dir"))
      pages_build_dir = v["build_dir"].asString();
  }
  if (root.isMember("cpp")) {
    auto v = root["cpp"];
    if (v.isMember("dir"))
      cpp_dir = v["dir"].asString();
    if (v.isMember("bin"))
      cpp_bin = v["bin"].asString();
  }
  if (root.isMember("resources")) {
    auto v = root["resources"];
    if (v.isMember("dir"))
      resources_dir = v["dir"].asString();
  }
}

Config CONFIG;
