#include <cassert>
#include <cctype>
#include <components.hpp>
#include <defines.hpp>
#include <parse.hpp>
#include <string_view>
#include <variant>

using namespace html;

Fragment html::parse(std::string s, components::Context *ctx) {
  if (s.size() == 0)
    return {};
  if (s[s.size() - 1] != '\0')
    s += '\0';
  std::string_view cursor = s;
  // check doctype for document ?
  return parse_fragment(cursor, ctx);
}

bool skip_whitespace(std::string_view &sv) {
  bool out = false;
  while (std::isspace(sv[0]) && sv[0] != '\0') {
    sv = sv.substr(1);
    out = true;
  }
  return out;
}

bool skip_comment_and_whitespace(std::string_view &sv) {
  bool was_skipped = false;
  while (true) {
    if (sv[0] == '\0')
      return was_skipped;
    if (std::isspace(sv[0])) {
      was_skipped = true;
      sv = sv.substr(1);
      continue;
    }
    if (sv.starts_with("<!--")) {
      sv = sv.substr(4);
      while (sv[0] != '\0' && !sv.starts_with("-->")) {
        sv = sv.substr(1);
      }
      if (sv.starts_with("-->")) {
        sv = sv.substr(3);
      }
      was_skipped = true;
      continue;
    }
    break;
  }
  return was_skipped;
}

Fragment html::parse_fragment(std::string_view &sv, components::Context *ctx) {
  Fragment out;
  while (true) {
    skip_comment_and_whitespace(sv);
    if (sv[0] == '\0')
      break;
    if (sv[0] != '<') {
      auto txt = read_all_text(sv);
      if (txt.size() > 0)
        out.push_back(txt);
      continue;
    }
    if (sv.starts_with("</") || sv[0] == '\0')
      break;
    auto f = parse_element(sv, ctx);
    out.insert(out.end(), f.begin(), f.end());
  }
  return out;
}

static std::string ident(std::string_view &sv, std::string_view end = "") {
  std::string out;
  while (true) {
    if (std::isspace(sv[0]) || sv[0] == '\0' || end.contains(sv[0]))
      break;
    out += sv[0];
    sv = sv.substr(1);
  }
  return out;
}

static std::string strlit(std::string_view &sv, std::string_view end = "") {
  std::string out;
  skip_whitespace(sv);
  char quote = 0;
  if (sv[0] == '"') {
    quote = '"';
    sv = sv.substr(1);
  } else if (sv[0] == '\'') {
    quote = '\'';
    sv = sv.substr(1);
  }
  auto strlit_end = [quote, end](char c) {
    if (quote == 0)
      return end.contains(c) || std::isspace(c);
    return c == quote;
  };
  while (true) {
    if (sv[0] == '\0' || strlit_end(sv[0]))
      break;
    out += sv[0];
    sv = sv.substr(1);
  }
  if (quote != 0)
    sv = sv.substr(1);
  return out;
}

static Fragment maybe_component(Element &&e, components::Context *ctx) {
  if (e.name == "Children")
    return {e};
  if (e.name[0] >= 'A' && e.name[0] <= 'Z') {
    return components::apply_to(std::move(e), ctx);
  } else {
    return {e};
  }
}

Fragment html::parse_element(std::string_view &sv, components::Context *ctx) {
  Element e;
  assert(sv[0] == '<' && sv[1] != '/');
  sv = sv.substr(1);
  e.name = ident(sv, "/>");
  assert(e.name.size() > 0);
  skip_whitespace(sv);
  assert(sv[0] != '\0');
  while (sv[0] != '/' && sv[0] != '>') {
    std::string attr_name = ident(sv, "/>=");
    std::optional<std::string> value = std::nullopt;
    skip_whitespace(sv);
    if (sv[0] == '=') {
      sv = sv.substr(1);
      skip_whitespace(sv);
      value = strlit(sv, "/>");
      skip_whitespace(sv);
    }
    e.attrs.insert(std::make_pair(attr_name, value));
  }
  assert(sv[0] == '/' || sv[0] == '>');
  if (sv[0] == '/') {
    e.self_close = true;
    assert(sv[1] == '>');
    sv = sv.substr(2);
    return maybe_component(std::move(e), ctx);
  }
  e.self_close = false;
  sv = sv.substr(1);
  e.children = parse_fragment(sv, ctx);
  assert(sv.starts_with("</"));
  sv = sv.substr(2);
  assert(sv.starts_with(e.name));
  sv = sv.substr(e.name.size());
  skip_whitespace(sv);
  assert(sv[0] == '>');
  sv = sv.substr(1);
  return maybe_component(std::move(e), ctx);
}

std::string html::read_all_text(std::string_view &sv) {
  std::string out;
  while (true) {
    u32 i = 0;
    while (sv[i] != '\0' && sv[i] != '<' && !std::isspace(sv[i])) {
      i++;
    }
    if (i != 0) {
      out += sv.substr(0, i);
      sv = sv.substr(i);
    }
    if (sv[0] == '\0')
      break;
    if (skip_comment_and_whitespace(sv)) {
      out += ' ';
      continue;
    }
    break;
  }
  if (out.ends_with(' ')) {
    out.pop_back();
  }
  return out;
}

Document html::parse_document(std::string s, components::Context *ctx) {
  if (s.size() == 0)
    return {};
  if (s.starts_with("<!DOCTYPE html>")) {
    s = s.substr(15);
  }
  if (s[s.size() - 1] != '\0')
    s += '\0';
  std::string_view cursor = s;

  skip_comment_and_whitespace(cursor);
  auto root = html::parse_element(cursor, ctx);
  html::Element root_elt;
  auto is_html_elt = [](Node const &n, std::string const &name) {
    return std::holds_alternative<Element>(n) &&
           std::get<Element>(n).name == name;
  };
  if (root.size() == 1 && is_html_elt(root[0], "html")) {
    root_elt = std::get<Element>(root[0]);
  } else {
    if (root.size() == 2 && is_html_elt(root[0], "head") &&
        is_html_elt(root[1], "body")) {
      root_elt = html::html("html", {}, {root});
    } else {
      root_elt = html::html(
          "html", {},
          {html::html("head", {}, {}, {}), html::html("body", {}, {}, {root})});
    }
  }
  return {.doctype = html::Doctype{}, .root_node = root_elt};
}
