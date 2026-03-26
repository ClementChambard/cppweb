#pragma once

#include "server_rendering_context.hpp"
#include <map>
#include <string>

namespace html {

std::string exec_compiled_format(ServerRenderingContext &ctx,
                                 std::string const &data);
std::string exec_component(ServerRenderingContext &ctx, std::string const &name,
                           std::map<std::string, std::string> const &attrs,
                           std::string const &inner_html);

} // namespace html
