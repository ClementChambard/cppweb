#include "html/macros.hpp"

START_SERVER_COMPONENT(TestServer)
DEF_INTERFACE(TestServer, true, true)
DEF_SERVER_EXEC(TestServer, params, inner_html) {
  (void)params, (void)inner_html;
  return "";
}
// TODO: exec
END_COMPONENT(TestServer)
