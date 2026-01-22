#include "components.hpp"
#include "defines.hpp"
#include "sys/logger.hpp"
#include "sys/read_file.hpp"
#include "http/resource.hpp"
#include <string_view>
#include <unordered_map>

namespace html {

using TagParams = std::unordered_map<std::string, std::string>;

struct Tag {
  bool is_component;
  bool is_closing;
  bool is_self_closing;
  std::string name;
  TagParams params;
};

static void skip_spaces(std::string_view &s) {
  if (s.size() == 0)
    return;
  u64 pos = 0;
  while (pos < s.size() && std::isspace(s[pos]))
    pos++;
  s = s.substr(pos);
}

static std::string_view read_balanced(std::string_view &s) {
  u32 n = 1;
  u64 pos = 0;
  while (true) {
    auto beg_pos = s.find("{{", pos);
    auto end_pos = s.find("}}", pos);
    if (beg_pos < end_pos) {
      n++;
      pos = beg_pos + 2;
    } else if (end_pos < beg_pos) {
      n--;
      pos = end_pos + 2;
      if (n == 0)
        break;
    } else if (end_pos == std::string_view::npos) {
      return "";
    } else {
      // error, should not happen
    }
  }
  auto res = s.substr(0, pos - 2);
  s = s.substr(pos);
  return res;
}

static std::string_view parse_ident(std::string_view &s) {
  u64 pos = 0;
  while (pos < s.size() &&
         ((s[pos] >= '0' && s[pos] <= '9') ||
          (s[pos] >= 'a' && s[pos] <= 'z') ||
          (s[pos] >= 'A' && s[pos] <= 'Z') || s[pos] == '_')) {
    pos++;
  }
  std::string_view ret;
  if (s.size() == pos) {
    ret = s;
  } else {
    ret = s.substr(0, pos);
  }
  s = s.substr(pos);
  return ret;
}

Tag parse_tag(std::string_view tag_strview) {
  Tag t;
  t.is_component = false;
  skip_spaces(tag_strview);
  if (tag_strview.size() == 0)
    return t;
  t.is_closing = false;
  if (tag_strview[0] == '/') {
    t.is_closing = true;
    tag_strview = tag_strview.substr(1);
    skip_spaces(tag_strview);
  }
  if (tag_strview.size() == 0 || tag_strview[0] != '{')
    return t;
  t.is_component = true;
  tag_strview = tag_strview.substr(1);
  skip_spaces(tag_strview);
  u64 pos = 0;
  while (pos < tag_strview.size() && tag_strview[pos] != '}' && !std::isspace(tag_strview[pos]))
    pos++;
  if (tag_strview.size() == pos) {
    t.name = tag_strview;
  } else {
    t.name = tag_strview.substr(0, pos);
  }
  tag_strview = tag_strview.substr(pos);
  skip_spaces(tag_strview);
  if (tag_strview.size() == 0 || tag_strview[0] != '}') {
    // ERROR:
    t.is_component = false;
    return t;
  }
  tag_strview = tag_strview.substr(1);
  skip_spaces(tag_strview);
  while (true) {
    if (tag_strview.size() == 0 || tag_strview.starts_with('/') || tag_strview.starts_with('>'))
      break;
    auto param_name = parse_ident(tag_strview);
    skip_spaces(tag_strview);
    if (!tag_strview.starts_with('=')) {
      t.params[std::string(param_name)] = "1";
      continue;
    }
    tag_strview = tag_strview.substr(1);
    skip_spaces(tag_strview);
    std::string param_value;
    if (!tag_strview.starts_with('"')) {
      // ERROR: TODO
    } else {
      tag_strview = tag_strview.substr(1);
      while (true) {
        if (tag_strview[0] == '"')
          break;
        if (tag_strview[0] == '\\') {
          tag_strview = tag_strview.substr(1);
        }
        param_value.push_back(tag_strview[0]);
        tag_strview = tag_strview.substr(1);
      }
    }
    tag_strview = tag_strview.substr(1); // final '"'
    t.params[std::string(param_name)] = param_value;
    skip_spaces(tag_strview);
  }
  t.is_self_closing = false;
  if (tag_strview.size() != 0 && tag_strview[0] == '/') {
    t.is_self_closing = true;
    tag_strview = tag_strview.substr(1);
    skip_spaces(tag_strview);
  }
  if (tag_strview.size() > 0) {
    // ERROR:
    t.is_component = false;
  }
  return t;
}

std::string component_string(char const *component_name,
                             TagParams const &params);

std::string replace_components(std::string_view &read_buf,
                               bool has_open_tag = false, Tag *open = nullptr) {
  std::string out_code;
  u64 pos = 0;
  u64 end_pos = 0;
  while ((pos = read_buf.find('<', 0)) != std::string_view::npos) {
    out_code += read_buf.substr(0, pos);
    end_pos = read_buf.find('>', pos);
    if (end_pos == std::string::npos) {
      sys::error("Invalid html: missing closing '>'");
      return out_code;
    }
    auto tag_s = read_buf.substr(pos, end_pos - pos + 1);
    auto tag = parse_tag(tag_s.substr(1, tag_s.size() - 2));
    if (!tag.is_component) {
      out_code += tag_s;
      read_buf = read_buf.substr(end_pos + 1);
      continue;
    }
    read_buf = read_buf.substr(end_pos + 1);
    if (tag.is_closing && tag.name == open->name) {
      return out_code;
    }
    if (!tag.is_self_closing) {
      tag.params["children"] = replace_components(read_buf, true, &tag);
    }
    out_code += component_string(tag.name.c_str(), tag.params);
  }
  out_code += read_buf;
  return out_code;
}

// fmt: normal '{{' ident '}}'
//      option '{{' ident '?' '}}'
//      replace_present '{{' ident '?' '{{' repl '}}' '}}'
//      replace_present_or '{{' ident '?' '{{' repl '}}' '?' '{{' or '}}' '}}'
struct ComponentPlaceholder {
  std::string_view name;
  std::string_view replace_with;
  std::string_view replace_with_alt;
  u64 length;
  bool is_optional = false;
  bool has_replace_with = false;
  bool has_replace_with_alt = false;
  bool has_error = false;
};

ComponentPlaceholder parse_placeholder(std::string_view code) {
  auto orig_cursor = code.data();
  ComponentPlaceholder ph;
  skip_spaces(code);
  ph.name = parse_ident(code);
  skip_spaces(code);
  if (!code.starts_with('?')) {
    if (!code.starts_with("}}")) {
      // ERROR:
      sys::info("ERROR: %s, %s, %d", std::string(ph.name).c_str(),
                std::string(code).c_str(), __LINE__);
      ph.has_error = true;
    }
    ph.length = code.data() - orig_cursor;
    return ph;
  }
  if (code.size() != 0) code = code.substr(1);
  skip_spaces(code);
  if (!code.starts_with('?')) {
    if (code.starts_with("}}")) {
      ph.is_optional = true;
      ph.length = code.data() - orig_cursor;
      return ph;
    }
    if (!code.starts_with("{{")) {
      // ERROR:
      sys::info("ERROR: %d", __LINE__);
      ph.has_error = true;
      return ph;
    }
    code = code.substr(2);
    ph.replace_with = read_balanced(code);
    ph.has_replace_with = true;
    skip_spaces(code);
    if (!code.starts_with('?')) {
      if (!code.starts_with("}}")) {
        // ERROR:
        sys::info("ERROR: %d", __LINE__);
        ph.has_error = true;
      }
      ph.length = code.data() - orig_cursor;
      return ph;
    }
  }
  if (code.size() != 0) code = code.substr(1);
  skip_spaces(code);
  if (!code.starts_with("{{")) {
    // ERROR:
    sys::info("ERROR: %d", __LINE__);
    ph.has_error = true;
    return ph;
  }
  code = code.substr(2);
  ph.replace_with_alt = read_balanced(code);
  ph.has_replace_with_alt = true;
  if (!code.starts_with("}}")) {
    // ERROR:
    sys::info("ERROR: %d", __LINE__);
    ph.has_error = true;
  }
  ph.length = code.data() - orig_cursor;
  return ph;
}

std::string component_string(char const *component_name,
                             TagParams const &params) {
  auto raw_html = http::Resource::get("html::" + std::string(component_name), true).get_res_contents();

  // 1st pass: apply params
  u64 pos = 0;
  while ((pos = raw_html.find("{{", pos)) != std::string::npos) {
    auto ph = parse_placeholder(std::string_view(raw_html).substr(pos + 2));
    if (ph.has_error) {
      sys::error("placeholder error in component: %s", component_name);
      pos++;
      continue;
    }
    auto it = params.find(std::string(ph.name));
    if (it == params.end()) {
      if (ph.is_optional) {
        raw_html.replace(pos, ph.length + 4, "");
      } else if (ph.has_replace_with_alt) {
        raw_html.replace(pos, ph.length + 4, ph.replace_with_alt);
      } else if (ph.has_replace_with) {
        raw_html.replace(pos, ph.length + 4, "");
      } else {
        pos++;
      }
      continue;
    }
    if (ph.has_replace_with) {
      raw_html.replace(pos, ph.length + 4, ph.replace_with);
    } else {
      raw_html.replace(pos, ph.length + 4, it->second);
    }
  }

#ifndef RELEASE
  // 2nd pass: apply components
  std::string_view html = raw_html;
  return replace_components(html);
#else
  return raw_html;
#endif
}

} // namespace html
