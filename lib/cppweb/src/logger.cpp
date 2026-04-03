#include "config.hpp"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <logger.hpp>
#include <set>
#include <sstream>
#include <unistd.h>

#include <iomanip>

namespace logger {

struct Config {
  bool colorized = false;
  bool log_to_file = false;
  Level min_level_console = Level::INFO;
  Level min_level = Level::INFO;
  std::string time_fmt = "%d-%m-%Y %H:%M:%S";
  std::string log_file = "console.log";
  std::set<std::string> extra_log_allowed{};
  struct color_t {
    struct {
      std::string name;
      std::string text;
    } lvl[int(Level::LEVEL_COUNT)];
    std::string time = "";
    std::optional<std::string> extra_kind = "";
  } colors;
} CONFIG{};

std::string color_code(std::string_view color_name) {
  // TODO: all colors
  if (color_name == "black")
    return "30";
  if (color_name == "red")
    return "31";
  if (color_name == "green")
    return "32";
  if (color_name == "orange")
    return "33";
  if (color_name == "blue")
    return "34";
  if (color_name == "purple")
    return "35";
  if (color_name == "cyan")
    return "36";
  if (color_name == "grey")
    return "37";
  return "";
}

std::string build_color_escape(bool bold, bool italic,
                               std::string_view color_name) {
  // TODO: more variations
  std::string seq;
  bool semi = false;
  if (bold) {
    if (semi)
      seq += ";";
    seq += "1";
    semi = true;
  }
  if (italic) {
    if (semi)
      seq += ";";
    seq += "3";
    semi = true;
  }
  auto code = color_code(color_name);
  if (color_name != "") {
    if (semi)
      seq += ";";
    seq += code;
  }
  return "\x1b[" + seq + "m";
}

void set_color(std::string &o_color, std::optional<std::string> const &value) {
  if (!value)
    return (void)(o_color = "");
  std::string_view cursor = *value;
  bool bold = false;
  bool italic = false;
  std::string_view color = "";
  while (true) {
    auto pos = cursor.find(' ');
    std::string_view part = cursor.substr(0, pos);
    if (part == "bold")
      bold = true;
    else if (part == "italic")
      italic = true;
    else
      color = part;

    if (pos == std::string_view::npos)
      break;
    cursor = cursor.substr(pos + 1);
  }
  o_color = build_color_escape(bold, italic, color);
}

void set_color(std::string &o_name, std::string &o_text,
               std::optional<std::string> const &value) {
  if (!value)
    return (void)(o_name = o_text = "");
  if (auto pos = value->find(';'); pos != std::string::npos) {
    set_color(o_name, value->substr(0, pos));
    set_color(o_text, value->substr(pos + 1));
  } else {
    set_color(o_name, value);
    set_color(o_text, value);
  }
}
void set_color(std::optional<std::string> &o_color,
               std::optional<std::string> const &value) {
  if (!value)
    return (void)(o_color = std::nullopt);
  o_color = "";
  set_color(*o_color, value);
}

void set_colors(::Config::logger_t::colors_t const &colors) {
  set_color(CONFIG.colors.lvl[int(Level::EXTRA)].name,
            CONFIG.colors.lvl[int(Level::EXTRA)].text, colors.extra);
  set_color(CONFIG.colors.lvl[int(Level::INFO)].name,
            CONFIG.colors.lvl[int(Level::INFO)].text, colors.info);
  set_color(CONFIG.colors.lvl[int(Level::WARN)].name,
            CONFIG.colors.lvl[int(Level::WARN)].text, colors.warn);
  set_color(CONFIG.colors.lvl[int(Level::ERROR)].name,
            CONFIG.colors.lvl[int(Level::ERROR)].text, colors.error);
  set_color(CONFIG.colors.lvl[int(Level::FATAL)].name,
            CONFIG.colors.lvl[int(Level::FATAL)].text, colors.fatal);
  set_color(CONFIG.colors.extra_kind, colors.extra_kind);
  set_color(CONFIG.colors.time, colors.time);
}

static i32 log_file_fd = -1;

void set_config(const ::Config::logger_t &config) {
  CONFIG.colorized = config.colorize_output.value_or(false);
  CONFIG.log_to_file = config.log_to_file.value_or(false);
  if (config.min_level) {
    if (*config.min_level == "info")
      CONFIG.min_level = Level::INFO;
    else if (*config.min_level == "warn")
      CONFIG.min_level = Level::WARN;
    else if (*config.min_level == "error")
      CONFIG.min_level = Level::ERROR;
    else if (*config.min_level == "fatal")
      CONFIG.min_level = Level::FATAL;
    else
      CONFIG.min_level = Level::INFO;
  } else {
    CONFIG.min_level = Level::INFO;
  }
  // TODO: separate ?
  CONFIG.min_level_console = CONFIG.min_level;
  CONFIG.extra_log_allowed.clear();
  CONFIG.extra_log_allowed.insert(config.log_extra.begin(),
                                  config.log_extra.end());
  auto old_log_file = CONFIG.log_file;
  CONFIG.log_file = config.log_file.value_or("console.log");

  if (config.colors)
    set_colors(*config.colors);

  if (log_file_fd != -1 &&
      (!CONFIG.log_to_file || CONFIG.log_file != old_log_file)) {
    close(log_file_fd);
    log_file_fd = -1;
  }

  if (CONFIG.log_to_file && log_file_fd == -1) {
    log_file_fd =
        open(CONFIG.log_file.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
  }
}

void append_to_log_file(char const *message) {
  u64 length = std::strlen(message);
  write(log_file_fd, message, length);
}

static char const *const level_strings[u64(Level::LEVEL_COUNT)] = {
    "EXTRA", "INFO", "WARN", "ERROR", "FATAL"};

void log_to_file_extra(const char *time, const char *extra,
                       const char *message) {
  std::ostringstream output;
  output << time << " [" << extra << "]: " << message;
  append_to_log_file(output.str().c_str());
}

void log_to_file(const char *time, Level lvl, const char *message) {
  std::ostringstream output;
  output << time << " [" << level_strings[int(lvl)] << "]: " << message << '\n';
  append_to_log_file(output.str().c_str());
}

struct ch_color {
  ch_color(std::string_view color) : color(color) {}
  std::string_view color;
  static std::string_view cur_color;
  static ch_color reset() { return {""}; }
};
std::string_view ch_color::cur_color;

std::ostream &operator<<(std::ostream &lhs, ch_color const &rhs) {
  if (!CONFIG.colorized || rhs.color == ch_color::cur_color)
    return lhs;
  lhs << "\x1b[0m";
  if (rhs.color != "")
    lhs << rhs.color;
  ch_color::cur_color = rhs.color;
  return lhs;
}

void log_to_console(const char *time, Level lvl, const char *message) {
  std::ostringstream oss;
  auto *stream = &std::cout;
  if (lvl >= Level::ERROR)
    stream = &std::cerr;
  *stream << ch_color(CONFIG.colors.time) << time
          << ch_color(CONFIG.colors.lvl[int(lvl)].name) << "["
          << level_strings[int(lvl)]
          << "]: " << ch_color(CONFIG.colors.lvl[int(lvl)].text) << message
          << ch_color::reset() << '\n';
}

void log_to_console_extra(const char *time, const char *extra,
                          const char *message) {
  std::ostringstream oss;
  auto *stream = &std::cout;

  std::string extra_kind_color = CONFIG.colors.lvl[int(Level::EXTRA)].name;
  if (CONFIG.colors.extra_kind)
    extra_kind_color = *CONFIG.colors.extra_kind;

  *stream << ch_color(CONFIG.colors.time) << time << ch_color(extra_kind_color)
          << "[" << extra
          << "]: " << ch_color(CONFIG.colors.lvl[int(Level::EXTRA)].text)
          << message << ch_color::reset() << '\n';
}

void log_inner(Level lvl, const char *extra, const char *message,
               va_list args) {
  bool to_file = CONFIG.log_to_file;
  bool to_console = true;
  if (lvl == Level::EXTRA) {
    if (std::find(CONFIG.extra_log_allowed.begin(),
                  CONFIG.extra_log_allowed.end(),
                  extra) == CONFIG.extra_log_allowed.end()) {
      return;
    }
  } else {
    to_file = CONFIG.log_to_file && CONFIG.min_level <= lvl;
    to_console = CONFIG.min_level_console <= lvl;
    if (!to_file && !to_console)
      return;
  }

  // TODO: how to make it better ?
  char message_buffer[16000] = {0};
  vsnprintf(message_buffer, sizeof(message_buffer), message, args);

  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);

  std::ostringstream oss;
  oss << std::put_time(&tm, CONFIG.time_fmt.c_str()) << ' ';
  std::string time_str = oss.str();

  if (to_file) {
    if (lvl == Level::EXTRA) {
      log_to_file_extra(time_str.c_str(), extra, message_buffer);
    } else {
      log_to_file(time_str.c_str(), lvl, message_buffer);
    }
  }
  if (to_console) {
    if (lvl == Level::EXTRA) {
      log_to_console_extra(time_str.c_str(), extra, message_buffer);
    } else {
      log_to_console(time_str.c_str(), lvl, message_buffer);
    }
  }
}

#define LOG_INNER(lvl, message)                                                \
  va_list args;                                                                \
  va_start(args, message);                                                     \
  logger::log_inner(lvl, nullptr, message, args);                              \
  va_end(args)

void info(char const *message, ...) { LOG_INNER(logger::Level::INFO, message); }

void warn(char const *message, ...) { LOG_INNER(logger::Level::WARN, message); }

void error(char const *message, ...) {
  LOG_INNER(logger::Level::ERROR, message);
}

void fatal_error(char const *message, ...) {
  LOG_INNER(logger::Level::FATAL, message);
  std::exit(EXIT_FAILURE);
}

#undef LOG_INNER

void log_extra(const char *extra, const char *message, ...) {
  va_list args;
  va_start(args, message);
  logger::log_inner(Level::EXTRA, extra, message, args);
  va_end(args);
}

} // namespace logger
