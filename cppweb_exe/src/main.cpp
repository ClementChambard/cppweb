#include "args.hpp"
#include "commands.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>

void expect_config() {
  if (!std::filesystem::exists("cppweb.conf")) {
    std::cerr << "ERROR: Not a cppweb project\n";
    std::exit(EXIT_FAILURE);
  }
}

int main(int argc, char **argv) {
  auto args = Args::parse(argc, argv);
  switch (args.action) {
  case Args::CREATE:
    create_project(args.create);
    break;
  case Args::RUN:
    expect_config();
    run(args.run);
    break;
  case Args::UPDATE:
    expect_config();
    update_cppweb();
    break;
  case Args::BUILD:
    expect_config();
    build_project();
    break;
  case Args::COMPILE:
    expect_config();
    compile_project();
    break;
  case Args::HELP:
    args.show_help();
    break;
  case Args::KILL:
    kill();
  }
  return 0;
}
