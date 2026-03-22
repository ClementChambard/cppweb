#include <handler.hpp>

namespace api {

void execute_handler_chain(const HandlerChain &chain, Context &ctx) {
  for (auto h : chain) {
    if (!h(ctx)) break;
  }
}

}
