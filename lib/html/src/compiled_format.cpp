#include "compiled_format.hpp"
#include "components.hpp"
#include "server_rendering_context.hpp"
#include <cassert>
#include <map>
#include <string_view>

namespace html {

static void skip_spaces(std::string_view &cursor) {
  while (std::isspace(cursor[0]))
    cursor = cursor.substr(1);
}

static std::string unescape(std::string_view in) { return std::string(in); }

static std::string parse_strlit(std::string_view &cursor) {
  auto pos = 0;
  while (true) {
    pos = cursor.find('"', pos);
    if (pos == 0 || cursor[pos - 1] != '\\')
      break;
    pos++;
  }
  auto out = unescape(cursor.substr(0, pos));
  cursor = cursor.substr(pos + 1);
  return out;
}

std::string exec_component(ServerRenderingContext &ctx, std::string const &name,
                           std::map<std::string, std::string> const &attrs,
                           std::string const &inner_html) {
  auto component = components::ServerComponent::find(name);
  if (component == nullptr) {
    return "&lt;UNKNOWN SERVER COMPONENT&gt;";
  }
  return component->exec(ctx, attrs, inner_html);
}

std::map<std::string, std::string> parse_attribs(std::string_view &cursor) {
  std::map<std::string, std::string> out;
  while (true) {
    skip_spaces(cursor);
    if (cursor[0] == '/' || cursor[0] == '>') {
      return out;
    }
    auto eq = cursor.find('=');
    auto name = cursor.substr(0, eq);
    cursor = cursor.substr(eq + 2);
    auto val = parse_strlit(cursor);
    out.insert(std::make_pair(name, val));
  }
  return out;
}

std::string to_component_end(ServerRenderingContext &ctx,
                             std::string_view &cursor,
                             std::string_view component_name);

std::string parse_component(ServerRenderingContext &ctx,
                            std::string_view &cursor) {
  auto end_of_name = cursor.find('}');
  std::string_view component_name = cursor.substr(0, end_of_name);
  cursor = cursor.substr(end_of_name + 1);
  auto attribs = parse_attribs(cursor);
  std::string inner_html;
  if (cursor[0] == '/' && cursor[1] == '>') {
    cursor = cursor.substr(2);
  } else {
    assert(cursor[0] == '>');
    cursor = cursor.substr(1);
    inner_html = to_component_end(ctx, cursor, component_name);
  }
  return exec_component(ctx, std::string(component_name), attribs, inner_html);
}

std::string to_component_end(ServerRenderingContext &ctx,
                             std::string_view &cursor,
                             std::string_view component_name) {
  std::string inner_html;
  auto next_component_open = cursor.find("<{");
  while (true) {
    auto next_component_close = cursor.find("</{");
    if (next_component_open < next_component_close) {
      inner_html += cursor.substr(0, next_component_open);
      cursor = cursor.substr(next_component_open + 2);
      inner_html += parse_component(ctx, cursor);
      next_component_open = cursor.find("<{");
    } else if (next_component_close < next_component_open) {
      inner_html += cursor.substr(0, next_component_close);
      cursor = cursor.substr(next_component_close + 3);
      auto close_pos = cursor.find("}>");
      assert(component_name == cursor.substr(0, close_pos));
      cursor = cursor.substr(close_pos + 2);
      return inner_html;
    } else {
      assert(next_component_open == std::string_view::npos);
      assert(next_component_close == std::string_view::npos);
      // TODO: should not happen.
      assert(false);
    }
  }
}

std::string exec_compiled_format(ServerRenderingContext &ctx,
                                 std::string const &data) {
  std::string out;
  std::string_view cursor = data;
  while (true) {
    auto next_component_open = cursor.find("<{");
    if (next_component_open == std::string_view::npos)
      break;
    out += cursor.substr(0, next_component_open);
    cursor = cursor.substr(next_component_open + 2);
    out += parse_component(ctx, cursor);
  }
  out += cursor;
  return out;
}

void ServerRenderingContext::get_special_html(std::string &s) {
  static auto pre = "<html><head><title>Error</title></head><body>";
  static auto post = "</body></html>";
  if (m_error) {
    s = pre + ("<h1>Error: " + m_param_str + "</h1>") + post;
  }
  if (m_redirect) {
    s = pre + ("<script>window.location='" + m_param_str + "';</script>") +
        post;
  }
}

std::optional<std::string>
ServerRenderingContext::get_cookie(std::string const &name) const {
  if (auto it = m_cookies.find(name); it != m_cookies.end()) {
    return it->second;
  }
  return std::nullopt;
}

static std::string_view trim(std::string_view s) {
  while (s.size() > 0 && std::isspace(s[0])) {
    if (s.size() <= 1)
      return s;
    s = s.substr(1);
  }
  while (s.size() > 0 && std::isspace(s[s.size() - 1])) {
    s = s.substr(0, s.size() - 1);
  }
  return s;
}

void ServerRenderingContext::set_cookies(std::string_view cursor) {
  while (true) {
    auto pos = cursor.find(';');
    std::string_view sub_cookies = cursor.substr(0, pos);
    auto eq = sub_cookies.find('=');
    auto cookie_name = trim(sub_cookies.substr(0, eq));
    auto cookie_value = trim(sub_cookies.substr(eq + 1));
    m_cookies.insert(
        std::make_pair(std::string(cookie_name), std::string(cookie_value)));

    if (pos == std::string_view::npos)
      break;
    cursor = cursor.substr(pos + 1);
  }
}

} // namespace html
