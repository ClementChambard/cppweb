#pragma once

#include "context.hpp"

namespace api {

using handler_fn = bool(Context&);

using HandlerChain = std::vector<handler_fn*>;

void execute_handler_chain(HandlerChain const &chain, Context &ctx);

};
