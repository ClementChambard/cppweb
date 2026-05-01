#include "page.hpp"
#include "component_library.hpp"
#include <cassert>
#include <cctype>
#include <cppweb/config.hpp>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <sys/logger.hpp>

std::string parse_and_compile_one(std::string_view &cursor);

struct ParsedTag {
  std::string name;
  std::map<std::string, std::optional<std::string>> attrs;
  bool has_body;
  std::string body;

  std::string to_string() {
    auto out = "<" + name;
    for (auto &[k, v] : attrs) {
      out += ' ' + k;
      if (v) {
        out += "=\"" + *v + '"';
      }
    }
    if (!has_body)
      return out + "/>";
    return out + ">" + body + "</" + name + ">";
  }

  std::string apply_component();
};

std::string get_ident(std::string_view &cursor) {
  std::string out;
  while (cursor[0] != '\0' && std::isspace(cursor[0]))
    cursor = cursor.substr(1);
  auto pred = [](char c) {
    bool isalphamin = c >= 'a' && c <= 'z';
    bool isalphamaj = c >= 'A' && c <= 'Z';
    bool isnum = c >= '0' && c <= '9';
    bool ispunct = c == '-' || c == '_';
    return isnum || ispunct || isalphamaj || isalphamin;
  };
  while (pred(cursor[0]) && cursor[0] != '\0') {
    out += cursor[0];
    cursor = cursor.substr(1);
  }
  return out;
}

std::string get_strlit(std::string_view &cursor) {
  while (cursor[0] != '\0' && std::isspace(cursor[0]))
    cursor = cursor.substr(1);
  char until[3] = {'\0', '/', '>'};
  bool until_space = true;
  if (cursor[0] == '"') {
    until[1] = until[2] = '"';
    cursor = cursor.substr(1);
    until_space = false;
  } else if (cursor[0] == '\'') {
    until[1] = until[2] = '\'';
    cursor = cursor.substr(1);
    until_space = false;
  }
  std::string out;
  while (true) {
    if (cursor[0] == until[0] || cursor[0] == until[1] || cursor[0] == until[2])
      break;
    if (until_space && std::isspace(cursor[0]))
      break;
    out += cursor[0];
    cursor = cursor.substr(1);
  }
  if (!until_space && cursor[0] != '\0')
    cursor = cursor.substr(1);
  return out;
}

ParsedTag parse_tag(std::string_view &cursor) {
  ParsedTag out{};
  assert(cursor[0] == '<');
  cursor = cursor.substr(1);
  out.name = get_ident(cursor);
  if (out.name == "") {
    return out;
  }
  out.has_body = true;
  while (true) {
    auto name = get_ident(cursor);
    if (name == "")
      break;
    while (std::isspace(cursor[0]) && cursor[0] != '\0')
      cursor = cursor.substr(1);
    if (cursor[0] != '=') {
      out.attrs.insert(std::make_pair(name, std::nullopt));
      continue;
    }
    cursor = cursor.substr(1);
    out.attrs.insert(std::make_pair(name, get_strlit(cursor)));
  }

  while (cursor[0] != '\0' && std::isspace(cursor[0]))
    cursor = cursor.substr(1);

  if (cursor[0] == '/') {
    out.has_body = false;
    cursor = cursor.substr(1);
  }

  while (cursor[0] != '\0' && std::isspace(cursor[0]))
    cursor = cursor.substr(1);

  assert(cursor[0] == '>');
  cursor = cursor.substr(1);

  if (!out.has_body)
    return out;

  while (true) {
    while (std::isspace(cursor[0]) && cursor[0] != '\0')
      cursor = cursor.substr(1);
    if (cursor[0] == '\0')
      return out;

    if (cursor.starts_with("</" + out.name))
      break;
    out.body += parse_and_compile_one(cursor);
  }
  auto pos = cursor.find('>');
  cursor = cursor.substr(pos + 1);
  return out;
}

// TODO: does this need to be better ?
std::string parse_and_compile_one(std::string_view &cursor) {
  if (cursor[0] != '<') {
    std::string out;
    while (cursor[0] != '<' && cursor[0] != '\0') {
      out += cursor[0];
      cursor = cursor.substr(1);
    }
    while (std::isspace(out[out.size() - 1]))
      out.pop_back();
    return out;
  }
  if (cursor[1] == '!' || cursor[1] == '/') { // comment or error
    cstr startwith = ">";
    u32 skip = 0;
    if (cursor[2] == '-' && cursor[3] == '-') {
      skip = 2;
      startwith = "-->";
    }
    cursor = cursor.substr(2 + skip);
    while (!cursor.starts_with(startwith) && cursor[0] != '\0')
      cursor = cursor.substr(1);

    if (cursor[0] != '\0')
      cursor = cursor.substr(1 + skip);
    return "";
  }
  bool is_component;
  if (cursor[1] >= 'a' && cursor[1] <= 'z') {
    is_component = false;
  } else if (cursor[1] >= 'A' && cursor[1] <= 'Z') {
    is_component = true;
  } else {
    std::string out = std::string(cursor.substr(0, 2));
    cursor = cursor.substr(2);
    return out;
  }
  auto tag = parse_tag(cursor);
  if (!is_component) {
    return tag.to_string();
  }
  return tag.apply_component();
}

std::string parse_and_compile(std::string input_html) {
  std::string_view cursor = {input_html.data(), input_html.size() + 1};
  std::string out;
  while (true) {
    while (std::isspace(cursor[0]) && cursor[0] != '\0')
      cursor = cursor.substr(1);
    if (cursor[0] == '\0')
      break;
    out += parse_and_compile_one(cursor);
  }
  return out;
}

std::string compile_file_with_children(std::string const &file_name,
                                       std::string children) {
  std::ifstream input(file_name);

  std::ostringstream oss;
  oss << input.rdbuf();
  std::string out = parse_and_compile(oss.str());
  auto pos = out.find("{{{CHILDREN}}}");
  if (pos != std::string::npos)
    out.replace(pos, sizeof("{{{CHILDREN}}}") - 1, children.c_str());
  return out;
}

void Page::compile() const {
  CUR_PAGE_DATA.reset();
  if (CONFIG.dev) {
    CUR_PAGE_DATA.js_to_include.push_back("/test.js");
  }

  std::string contents;

  for (u32 i = 0; i < render_path.size(); i++) {
    contents = compile_file_with_children(render_path[i], contents);
  }

  std::string head;
  if (CUR_PAGE_DATA.page_title == "") {
    head = "<title>cppweb page</title>";
  } else {
    head = "<title>" + CUR_PAGE_DATA.page_title + "</title>";
  }
  std::set<std::string> included;
  for (auto const &s : CUR_PAGE_DATA.css_to_include) {
    bool inserted = included.insert(s).second;
    if (inserted)
      head += "<link rel=\"stylesheet\" href=\"" + s + "\"/>";
  }
  included.clear();
  for (auto const &s : CUR_PAGE_DATA.js_to_include) {
    bool inserted = included.insert(s).second;
    if (inserted)
      head += "<script src=\"" + s + "\" defer></script>";
  }

  std::ofstream out(output_filename);
  out << "<!DOCTYPE html><html><head>" + head + "</head><body>" + contents +
             "</body></html>";
}

void find_pages_it(std::string dir_name, std::string cur_path,
                   std::vector<Page> &pages,
                   std::vector<std::string> &cur_render_path) {
  std::vector<std::filesystem::path> directories_to_visit;
  bool has_layout = false;
  bool has_page = false;
  for (auto const &ent : std::filesystem::directory_iterator(dir_name)) {
    if (ent.is_directory()) {
      directories_to_visit.push_back(ent.path());
      continue;
    }
    if (ent.path().filename() == "page.html") {
      has_page = true;
    }
    if (ent.path().filename() == "layout.html") {
      has_layout = true;
    }
  }
  if (has_layout) {
    cur_render_path.push_back(dir_name + "/layout.html");
  }
  if (has_page) {
    sys::log_extra("COMPILE", "page %s.html", cur_path.c_str());
    pages.push_back(Page{});
    pages.back().output_filename = cur_path + ".html";
    pages.back().render_path.push_back(dir_name + "/page.html");
    pages.back().render_path.insert(pages.back().render_path.end(),
                                    cur_render_path.rbegin(),
                                    cur_render_path.rend());
  }
  for (auto const &d : directories_to_visit) {
    auto name = d.filename().string();
    std::string fname = name;
    if (fname.starts_with("[...") && fname.ends_with(']')) {
      fname = "a" + std::to_string(fname.size() - 5) +
              fname.substr(4, fname.size() - 5);
    } else if (fname.starts_with('[') && fname.ends_with(']')) {
      fname = "p" + std::to_string(fname.size() - 2) +
              fname.substr(1, fname.size() - 2);
    } else if (fname.starts_with('(') && fname.ends_with(')')) {
      fname = "";
    } else {
      fname = std::to_string(fname.size()) + fname;
    }
    find_pages_it(dir_name + "/" + name, cur_path + fname, pages,
                  cur_render_path);
  }
  if (has_layout) {
    cur_render_path.pop_back();
  }
}

std::vector<Page> Page::find_all(std::string const &input_dir,
                                 std::string const &output_dir) {
  std::vector<Page> pages;

  std::vector<std::string> cur_path;
  find_pages_it(input_dir, output_dir + "/_", pages, cur_path);

  // TODO: check if 2 pages have the same path

  return pages;
}

void build_all_pages(std::string const &input_dir,
                     std::string const &output_dir) {
  auto pages = Page::find_all(input_dir, output_dir);

  for (auto const &p : pages) {
    p.compile();
  }
}

bool try_apply_attr_str(cstr &val, std::optional<std::string> const &value) {
  if (value)
    val = value->c_str();
  else
    val = nullptr;
  return true;
}

bool try_apply_attr_bool(long long int &val,
                         std::optional<std::string> const &value) {
  if (!value || value == "0" || value == "false")
    val = 0;
  else
    val = 1;
  return true;
}

bool try_apply_attr_int(long long int &val,
                        std::optional<std::string> const &value) {
  if (!value)
    return false;
  try {
    val = std::stoll(*value);
  } catch (std::exception *e) {
    return false;
  }
  return true;
}

bool try_apply_attr_float(double &val,
                          std::optional<std::string> const &value) {
  if (!value)
    return false;
  try {
    val = std::stod(*value);
  } catch (std::exception *e) {
    return false;
  }
  return true;
}

bool try_apply_attr(CPPWEB_Val &out, std::optional<std::string> const &value,
                    CPPWEB_StaticComponentAttrDecl const &attr) {
  if (attr.type == CPPWEB_ATTR_STRING && !try_apply_attr_str(out.str, value))
    return false;
  if (attr.type == CPPWEB_ATTR_BOOL &&
      !try_apply_attr_bool(out.number_i, value))
    return false;
  if (attr.type == CPPWEB_ATTR_INTEGER &&
      !try_apply_attr_int(out.number_i, value))
    return false;
  if (attr.type == CPPWEB_ATTR_FLOAT &&
      !try_apply_attr_float(out.number_f, value))
    return false;

  if (attr.validator)
    return attr.validator(out);
  return true;
}

bool try_apply_attr(std::string const &name,
                    std::optional<std::string> const &value,
                    CPPWEB_Val *declared_args, bool *set_args,
                    CPPWEB_StaticComponentArgList &arg_list,
                    CPPWEB_StaticComponentDecl const *component) {
  i32 declared_i = -1;
  for (u32 i = 0; i < component->attrs_count; i++) {
    if (name == component->attrs[i].name) {
      declared_i = i;
      break;
    }
  }
  if (declared_i == -1) {
    arg_list.names[arg_list.count] = name.c_str();
    arg_list.values[arg_list.count] = value ? value->c_str() : nullptr;
    return true;
  }
  set_args[declared_i] = true;
  return try_apply_attr(declared_args[declared_i], value,
                        component->attrs[declared_i]);
}

std::string ParsedTag::apply_component() {
  auto component = ComponentLibrary::find(name.c_str());
  if (component == nullptr) {
    return "{UNKNOWN COMPONENT: " + name + "}";
  }

  if (has_body && !component->can_have_body) {
    return "{COMPONENT ERROR: " + name + " CANT HAVE BODY}";
  }

  // TODO: more than 32 ?
  CPPWEB_Val declared_args[32];
  bool set_args[32];
  cstr remaining_name[32];
  cstr remaining_val[32];
  std::memset(declared_args, 0, sizeof(declared_args));
  std::memset(set_args, 0, sizeof(set_args));
  std::memset(remaining_name, 0, sizeof(remaining_name));
  std::memset(remaining_val, 0, sizeof(remaining_val));
  CPPWEB_StaticComponentArgList remaining_args{0, remaining_name,
                                               remaining_val};
  CPPWEB_StaticComponentBody body = {this->body};

  for (auto const &[k, v] : attrs) {
    if (!try_apply_attr(k, v, declared_args, set_args, remaining_args,
                        component)) {
      return "{COMPONENT ERROR: " + name + " INVALID ARG: " + k + "}";
    }
  }

  for (u32 i = 0; i < component->attrs_count; i++) {
    if (set_args[i])
      continue;
    if (!component->attrs[i].optional)
      return "{COMPONENT ERROR: " + name + " MISSING ATTR " +
             component->attrs[i].name + "}";
    declared_args[i] = component->attrs[i].default_value;
  }

  if (remaining_args.count > 0 && !component->can_have_more_attrs) {
    return "{COMPONENT ERROR: " + name + " CANT HAVE MORE ATTR THAN DEFINED}";
  }

  if (component->render_func == nullptr) {
    std::string out = "<{" + name + "}";
    for (u32 i = 0; i < remaining_args.count; i++) {
      out += std::string(" ") + remaining_args.names[i] + "=\"" +
             (remaining_args.values[i] ? remaining_args.values[i] : "") + "\"";
    }
    out += ">" + body.content + "</{" + name + "}>";
    return out;
  }

  return component->render_func({declared_args, remaining_args, body});
}
