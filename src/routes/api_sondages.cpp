#include "../db.hpp"
#include "routes.hpp"

namespace api::sondages {

http::Response get(http::Request r) {
  Json::Value out{Json::ValueType::arrayValue};
  ITER_DB(s, id, SondagesDb) {
    Json::Value o{Json::ValueType::objectValue};
    o["id"] = s->id;
    o["active"] = s->active;
    o["name"] = s->name;
    o["desc"] = s->desc;
    o["route"] = s->route;
    o["btn_text"] = s->button_text;
    out.append(o);
  }
  return http::Response::Builder()
      .code(200)
      .json(out)
      .close()
      .build();
}

http::Response put(http::Request r) {
  if (!is_authentified(r))
    return http::Response::unauthorized();
  auto v = r.body_as_json();
  if (v == std::nullopt) return http::Response::bad_request();
  auto body = *v;
  if (!body.isObject()) return http::Response::bad_request();

  SondagesDb::lock();
  auto sondage = SondagesDb::get().get_id(r.int_param("id", -1));
  if (!sondage) {
    SondagesDb::unlock();
    return http::Response::not_found();
  }
  sondage->active = body.get_bool("active", sondage->active);
  sondage->name = body.get_string("name", sondage->name.c_str());
  sondage->button_text = body.get_string("btn_text", sondage->button_text.c_str());
  sondage->desc = body.get_string("desc", sondage->desc.c_str());
  sondage->route = body.get_string("route", sondage->route.c_str());
  SondagesDb::get().write();
  SondagesDb::unlock();
  return http::Response::ok();
}

} // namespace api::sondages
