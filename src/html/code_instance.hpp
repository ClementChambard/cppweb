#ifndef IG_HTML_CODE_INSTANCE_HPP
#define IG_HTML_CODE_INSTANCE_HPP

#include <string>

namespace html {

std::string get_code_instance(char const *html_name, bool no_componentation=false);

void cleanup_cache();

} // namespace html

#endif // !IG_HTML_CODE_INSTANCE_HPP
