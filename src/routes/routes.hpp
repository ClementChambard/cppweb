#ifndef IG_ROUTES_HPP
#define IG_ROUTES_HPP

#include "http/request.hpp"
#include "http/response.hpp"

bool is_authentified(http::Request r);

namespace api {

http::Response check_admin(http::Request r);

namespace sondages {
http::Response get(http::Request r);
http::Response put(http::Request r);
} // namespace sondages

namespace piscine {
http::Response get(http::Request r);
http::Response put(http::Request r);
http::Response post(http::Request r);
http::Response del(http::Request r);
} // namespace piscine

namespace rdvs {
http::Response get(http::Request r);
http::Response put(http::Request r);
http::Response post(http::Request r);
http::Response del(http::Request r);
} // namespace rdvs

namespace rollers {
http::Response get(http::Request r);
http::Response put(http::Request r);
http::Response post(http::Request r);
http::Response del(http::Request r);
} // namespace rollers

} // namespace api

#endif // !IG_ROUTES_HPP
