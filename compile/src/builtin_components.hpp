
#include <cppweb_static_component.hpp>

#define DECLARE_BUILTIN(name)                                                  \
  extern CPPWEB_StaticComponentDecl CPPWEB_##name##_DECL

DECLARE_BUILTIN(ServerComponent);
DECLARE_BUILTIN(UseJs);
DECLARE_BUILTIN(UseCss);
DECLARE_BUILTIN(PageTitle);
DECLARE_BUILTIN(Children);
