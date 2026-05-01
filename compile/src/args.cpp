#include "args.hpp"
#include <iostream>

void Args::usage() {
  std::cerr << "Usage:\n"
            << prog_name
            << " <component_library> <input_file> [options]\n"
               "Options:\n"
               "  -c <config_file> : use config file\n"
               "  -h               : show this message\n"
               "  -o <output_file> : sets the output file name\n"
               "  -d               : directory mode\n"
               "  -gz              : gzips the output\n"
               "  -dev             : development mode\n";
}

std::string Args::next(int &argc, char **&argv) {
  if (argc == 0) {
    std::cerr << "missing argument\n";
    usage();
    std::exit(EXIT_FAILURE);
  }
  argc--;
  return *argv++;
}

void Args::parse(int argc, char **argv) {
  prog_name = next(argc, argv);
  while (argc > 0) {
    auto arg = next(argc, argv);
    if (arg == "-d") {
      mode = DIRECTORY;
      if (output_name == "out.html")
        output_name = "";
    } else if (arg == "-dev") {
      dev = true;
    } else if (arg == "-h") {
      usage();
      std::exit(EXIT_SUCCESS);
    } else if (arg == "-gz") {
      gzipped = true;
    } else if (arg == "-o") {
      output_name = next(argc, argv);
    } else if (arg == "-c") {
      config_name = next(argc, argv);
    } else if (component_library == "") {
      component_library = arg;
    } else if (input_name == "") {
      input_name = arg;
    } else {
      std::cerr << "invalid argument\n";
      usage();
      std::exit(EXIT_FAILURE);
    }
  }

  // verify args
  if (config_name != "") {
    return;
  }
  if (component_library == "") {
    std::cerr << "missing component library\n";
    usage();
    std::exit(EXIT_FAILURE);
  }
  if (input_name == "") {
    std::cerr << "missing input\n";
    usage();
    std::exit(EXIT_FAILURE);
  }
  if (output_name == "") {
    std::cerr << "missing output\n";
    usage();
    std::exit(EXIT_FAILURE);
  }
}
