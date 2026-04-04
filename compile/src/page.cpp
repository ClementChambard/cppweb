#include "page.hpp"
#include "html/components.hpp"
#include "sys/logger.hpp"
#include "ws_connection.hpp"
#include <cppweb/config.hpp>
#include <filesystem>
#include <fstream>
#include <html/html.hpp>
#include <html/parse.hpp>
#include <iostream>
#include <sstream>

void apply_children(html::Fragment &f, html::Fragment const &children) {
  for (u32 i = 0; i < f.size(); i++) {
    if (!std::holds_alternative<html::Element>(f[i]))
      continue;
    auto &e = std::get<html::Element>(f[i]);
    if (e.name == "Children") {
      f.erase(f.begin() + i);
      f.insert(f.begin() + i, children.begin(), children.end());
      continue;
    }
    if (e.children.size() > 0)
      apply_children(e.children, children);
  }
}

html::Fragment compile_file_with_children(std::string const &file_name,
                                          html::Fragment const &children,
                                          components::Context *ctx) {
  (void)children;
  std::ifstream input(file_name);

  std::ostringstream oss;
  oss << input.rdbuf();
  auto frag = html::parse(oss.str(), ctx);
  apply_children(frag, children);
  return frag;
}

html::Document compile_document_with_children(std::string const &file_name,
                                              html::Fragment const &children,
                                              components::Context *ctx) {
  (void)children;
  std::ifstream input(file_name);

  std::ostringstream oss;
  oss << input.rdbuf();
  auto content = html::parse_document(oss.str(), ctx);

  // TODO: document specific stuff

  apply_children(content.root_node.children, children);

  return content;
}

void Page::compile() const {
  html::Fragment contents = {};
  components::Context ctx;
  for (u32 i = 0; i < render_path.size() - 1; i++) {
    contents = compile_file_with_children(render_path[i], contents, &ctx);
  }
  auto document =
      compile_document_with_children(render_path.back(), contents, &ctx);

  if (CONFIG.dev) {
    ws_add_script_to_document(document);
  }
  auto &head = std::get<html::Element>(document.root_node.children[0]);
  for (auto const &s : ctx.scripts_to_include) {
    head.children.push_back(
        html::html("script", {{"src", s}, {"defer", std::nullopt}}, {}, {}));
  }
  for (auto const &s : ctx.css_to_link) {
    head.children.push_back(
        html::html("link", {{"href", s}, {"rel", "stylesheet"}}));
  }

  std::ofstream out(output_filename);
  out << html::node_str(document);
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

std::vector<Page> Page::find_all() {
  std::vector<Page> pages;

  std::vector<std::string> cur_path;
  find_pages_it(CONFIG.pages.dir, CONFIG.pages.build_dir + "/_", pages,
                cur_path);

  // TODO: check if 2 pages have the same path

  return pages;
}
