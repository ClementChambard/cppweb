#pragma once

#include "html.hpp"

namespace html {

Document parse_document(std::string s);

Fragment parse(std::string s);

Fragment parse_fragment(std::string_view &sv);

Fragment parse_element(std::string_view &sv);

std::string read_all_text(std::string_view &sv);

} // namespace html
