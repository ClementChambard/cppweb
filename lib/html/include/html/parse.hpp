#pragma once

#include "components.hpp"
#include "html.hpp"

namespace html {

Document parse_document(std::string s, components::Context *ctx);

Fragment parse(std::string s, components::Context *ctx);

Fragment parse_fragment(std::string_view &sv, components::Context *ctx);

Fragment parse_element(std::string_view &sv, components::Context *ctx);

std::string read_all_text(std::string_view &sv);

} // namespace html
