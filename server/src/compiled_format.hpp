#pragma once

#include <cppweb/server_rendering_context.hpp>
#include <map>
#include <string>

std::string exec_compiled_format(cppweb::ServerRenderingContext &ctx,
                                 std::string const &data);
std::string exec_component(cppweb::ServerRenderingContext &ctx,
                           std::string const &name,
                           std::map<std::string, std::string> const &attrs,
                           std::string const &inner_html);
