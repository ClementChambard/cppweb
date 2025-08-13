#ifndef IG_ROUTES_HPP
#define IG_ROUTES_HPP

#include "http/request.hpp"
#include "http/response.hpp"

std::string format_date(std::string date);
bool is_authentified(http::Request r);

namespace page {

std::string root(http::Request);
std::string sondage_rdvs(http::Request r);
std::string sondage_rollers(http::Request r);
std::string sondage_piscine(http::Request r);
std::string admin_sondages(http::Request r);
std::string admin_sondages_rdvs(http::Request r);
std::string admin_sondages_rollers(http::Request r);
std::string admin_sondages_piscine(http::Request r);

} // namespace page

namespace api {

http::Response check_admin(http::Request r);
http::Response sondages_id(http::Request r);
http::Response rdvs_id(http::Request r);
http::Response rdvs(http::Request r);
http::Response rollers_id(http::Request r);
http::Response rollers(http::Request r);
http::Response piscine_id(http::Request r);
http::Response piscine(http::Request r);

} // namespace api

#endif // !IG_ROUTES_HPP
