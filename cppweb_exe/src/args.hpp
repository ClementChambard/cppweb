#pragma once

#include <optional>
#include <string>

struct Args {
  std::string_view program_name;

  enum Action {
    CREATE,
    RUN,
    UPDATE,
    BUILD,
    COMPILE,
    HELP,
    KILL,
  } action;

  // create args
  struct Create {
    std::string_view project_name;
    std::optional<std::string_view> builder;
    std::optional<std::string_view> build_dir;
  } create{};

  // run args
  enum RunMode {
    NORMAL,
    DEV,
  };
  struct Run {
    std::optional<std::string_view> port = std::nullopt;
    RunMode run_mode = NORMAL;
  } run{};

  // update args
  struct Update {
    bool bin_only = false;
  } update{};

  static Args parse(int argc, char **argv);

  static void show_help(std::string_view subcommand = "");
  static std::string_view run_mode_str(RunMode r);
  static std::string_view action_str(Action a);

private:
  static std::string_view shift(int &argc, char **&argv);
  static void error_message(std::string const &msg);
  static Action parse_action(std::string_view action_name);
  static RunMode parse_run_mode(std::string_view run_mode_name);
  void parse_create_args(int &argc, char **&argv);
  void parse_run_args(int &argc, char **&argv);
  void parse_update_args(int &argc, char **&argv);
};
