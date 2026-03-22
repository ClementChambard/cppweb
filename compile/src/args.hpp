#pragma once

#include <string>

struct Args {
  void parse(int argc, char **argv);
  std::string next(int &argc, char **&argv);
  void usage();

  enum Mode {
    FILE,
    DIRECTORY,
  } mode = FILE;

  std::string prog_name = "";
  std::string input_name = "";
  std::string output_name = "out.html";
  std::string component_library = "";
  std::string config_name = "";
  bool gzipped = false;
  bool dev = false;
};
