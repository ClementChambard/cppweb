#include "args.hpp"
#include <cstdlib>
#include <iostream>
#include <optional>

Args Args::parse(int argc, char **argv) {
  Args out;
  out.program_name = shift(argc, argv);
  if (argc == 0) {
    show_help();
    std::exit(EXIT_FAILURE);
  }
  out.action = parse_action(shift(argc, argv));
  switch (out.action) {
  case CREATE:
    out.parse_create_args(argc, argv);
    break;
  case RUN:
    out.parse_run_args(argc, argv);
    break;
  case HELP:
    if (argc > 0) {
      show_help(shift(argc, argv));
      std::exit(EXIT_SUCCESS);
    }
  default:
    break;
  }

  if (argc > 0) {
    error_message("too many arguments");
    show_help(action_str(out.action));
    std::exit(EXIT_FAILURE);
  }

  return out;
}

void Args::show_help(std::string_view subcommand) {
  if (subcommand == "help") {
    std::cout << "cppweb help [subcommand]\n"
                 "  shows help message\n\n"
                 "Args:\n"
                 "  subcommand   Which subcommand to get help about\n";
  } else if (subcommand == "kill") {
    std::cout << "cppweb kill\n"
                 "  kills running sockets\n\n"
                 "If the server crashes, some sockets might remain open for a "
                 "bit. This command closes them.\n"
                 "It needs to be ran as root.\n";
  } else if (subcommand == "compile") {
    std::cout
        << "cppweb compile\n"
           "  compiles the c++ library\n\n"
           "This command only compiles the c++ library of your application. "
           "This can be useful if only c++ code has changed.\n";
  } else if (subcommand == "build") {
    std::cout << "cppweb build\n"
                 "  builds the project\n\n"
                 "This command builds the entire project.\n";
  } else if (subcommand == "update") {
    std::cout << "cppweb update\n"
                 "  updates the cppweb framework\n\n"
                 "This command recreates the runtime data.\n"
                 "This can be useful if you recompiled the cppweb libraries "
                 "and want your project to use these new versions.\n";
  } else if (subcommand == "run") {
    std::cout
        << "cppweb run [run-profile] [-p <port>]\n"
           "  runs the project\n\n"
           "Args:\n"
           "  run-profile   Only allowed value (for now) is: 'dev'\n"
           "  -p <port>     Set the port on which the http server will run\n\n"
           "This command sets up everything to run the project.\n"
           "When choosing 'dev' run profile, a separate development server "
           "will be launched to enable hot recompiling and hot reloading of "
           "your code.\n";
  } else if (subcommand == "create") {
    std::cout << "cppweb create <project-name> [args...]\n"
                 "  creates a new cppweb project\n\n"
                 "Args:\n"
                 "  project-name  Name of the new project\n"
                 "  -G???         Select the builder (Makefile/Ninja)\n"
                 "  -B <dir>      Select the name of the build directory\n\n"
                 "This will create a new directory with a project template.\n";
  } else {
    std::cout <<
        R"help(cppweb <subcommand> [args...]
  The c++ web framework

Subcommands:
 - create    creates a new cppweb project
 - run       runs the project
 - update    updates the cppweb framework
 - build     builds the project
 - compile   compiles the c++ library
 - help      shows help message
 - kill      kills running sockets

Type cppweb help [subcommand] for some more information
)help";
  }
}

std::string_view Args::run_mode_str(RunMode r) {
  if (r == DEV)
    return "dev";
  return "";
}

std::string_view Args::action_str(Action a) {
  if (a == CREATE)
    return "create";
  if (a == RUN)
    return "run";
  if (a == UPDATE)
    return "update";
  if (a == BUILD)
    return "build";
  if (a == COMPILE)
    return "compile";
  if (a == HELP)
    return "help";
  if (a == KILL)
    return "kill";
  return "";
}

std::string_view Args::shift(int &argc, char **&argv) {
  std::string_view out = *argv;
  argc--;
  argv++;
  return out;
}

void Args::error_message(std::string const &msg) { std::cerr << msg << '\n'; }

Args::Action Args::parse_action(std::string_view action_name) {
  if (action_name == "create")
    return CREATE;
  if (action_name == "run")
    return RUN;
  if (action_name == "update")
    return UPDATE;
  if (action_name == "build")
    return BUILD;
  if (action_name == "compile")
    return COMPILE;
  if (action_name == "help")
    return HELP;
  if (action_name == "kill")
    return KILL;
  error_message("unknown action " + std::string(action_name));
  show_help();
  std::exit(EXIT_FAILURE);
}

Args::RunMode Args::parse_run_mode(std::string_view run_mode_name) {
  if (run_mode_name == "dev")
    return DEV;
  error_message("unknown run mode " + std::string(run_mode_name));
  show_help("run");
  std::exit(EXIT_FAILURE);
}

void Args::parse_create_args(int &argc, char **&argv) {
  bool has_project_name = false;
  while (argc > 0) {
    auto arg = shift(argc, argv);
    if (arg.starts_with("-G")) {
      if (create.builder != std::nullopt) {
        error_message("duplicate flag: -G");
        show_help("create");
        std::exit(EXIT_FAILURE);
      }
      create.builder = arg;
    } else if (arg == "-B") {
      if (create.build_dir != std::nullopt) {
        error_message("duplicate flag: -B");
        show_help("create");
        std::exit(EXIT_FAILURE);
      }
      if (argc == 0) {
        error_message("missing directory name after -B");
        show_help("create");
        std::exit(EXIT_FAILURE);
      }
      create.build_dir = shift(argc, argv);
    } else {
      if (has_project_name) {
        error_message("duplicate project name");
        show_help("create");
        std::exit(EXIT_FAILURE);
      }
      has_project_name = true;
      create.project_name = arg;
    }
  }
  if (!has_project_name) {
    error_message("missing project name");
    show_help("create");
    std::exit(EXIT_FAILURE);
  }
}

void Args::parse_run_args(int &argc, char **&argv) {
  bool has_port = false;
  bool has_mode = false;
  while ((!has_mode || !has_port) && argc > 0) {
    auto arg = shift(argc, argv);
    if (arg == "-p") {
      if (argc == 0 || has_port) {
        error_message("missing port after -p");
        show_help("run");
        std::exit(EXIT_FAILURE);
      }
      run.port = shift(argc, argv);
      has_port = true;
    } else if (!has_mode) {
      run.run_mode = parse_run_mode(arg);
      has_mode = true;
    }
  }
}
