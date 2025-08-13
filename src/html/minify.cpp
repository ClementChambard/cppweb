#include <algorithm>
#include <string>
#include <defines.hpp>
#include <string_view>
#include "./parse_help.hpp"

namespace html {

void minify_tag(std::string_view &input, std::string &output) {
  output += '<';
  skip_1(input);
  skip_spaces(input);
  if (input.starts_with('/')) {
    output += '/';
    skip_1(input);
    skip_spaces(input);
  }
  if (input.starts_with('{')) {
    // component: do not minify
    u64 pos = 0;
    while (input[pos] != '>') {
      pos++;
      if (pos == input.size()) {
        output += input;
        input = "";
        return;
      }
    }
    output += input.substr(0, pos + 1);
    input = input.substr(pos + 1);
    return;
  }
  output += get_until_space_or(input, "/>");
  skip_spaces(input);
  while (!input.starts_with('>') && !input.starts_with('/')) {
    output += ' ';
    output += get_until_space_or(input, "=/>");
    skip_spaces(input);
    if (input.starts_with('=')) {
      output += '=';
      skip_1(input);
      skip_spaces(input);
      if (!input.starts_with('"')) {
        output += get_until_space_or(input, "/>");
      } else {
        u64 pos = 1;
        while (input[pos] != '"') pos++;
        output += input.substr(0, pos + 1);
        input = input.substr(pos + 1);
      }
      skip_spaces(input);
    }
  }
  if (input.starts_with('/')) {
    output += '/';
    skip_1(input);
    skip_spaces(input);
  }
  if (input.starts_with('>')) {
    output += '>';
    skip_1(input);
  }
  return;
}

void minify_text(std::string_view &input, std::string &output) {
  while (std::isspace(input[0])) {
    input = input.substr(1);
    if (input.size() == 0) return;
  }
  while (input.size() > 0) {
    u64 pos = 0;
    while (std::isspace(input[pos])) {
      pos++;
      if (input.size() == pos) return;
    }
    if (input[pos] == '<') {
      input = input.substr(pos);
      return;
    } else {
      output += input.substr(0, pos + 1);
      input = input.substr(pos + 1);
    }
  }
}

std::string minify(std::string_view input) {
  std::string output;
  output.reserve(std::max(u64(input.size() / 4), u64(8)));

  while (input.size() > 0) {
    if (input[0] == '<') {
      minify_tag(input, output);
    } else {
      minify_text(input, output);
    }
  }

  return output;
}

} // namespace html
