#pragma once

#include <defines.hpp>
#include <string>
#include <vector>

struct Page {
  std::string output_filename;
  std::vector<std::string> render_path;

  void compile() const;

  static std::vector<Page> find_all(std::string const &input_dir,
                                    std::string const &output_dir);
};

void build_all_pages(std::string const &input_dir,
                     std::string const &output_dir);
