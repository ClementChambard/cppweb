#include "../db.hpp"
#include "routes.hpp"
#include "sys/logger.hpp"

namespace api::piscine {

http::Response get(http::Request r) {
  Json::Value out{Json::ValueType::arrayValue};
  ITER_DB(s, id, PiscineDb) {
    Json::Value o{Json::ValueType::objectValue};
    o["id"] = s->id;
    o["date"] = s->date;
    o["parent"] = s->parent;
    out.append(o);
  }
  return http::Response::Builder()
      .code(200)
      .json(out)
      .close()
      .build();
}

http::Response put(http::Request r) {
  bool auth = is_authentified(r);
  auto v = r.body_as_json();
  if (v == std::nullopt) return http::Response::bad_request();
  auto body = *v;
  if (!body.isObject()) return http::Response::bad_request();

  PiscineDb::lock();
  auto piscine = PiscineDb::get().get_id(r.int_param("id", -1));
  if (!piscine) {
    PiscineDb::unlock();
    return http::Response::not_found();
  }
  if (auth || piscine->parent == "") {
    piscine->parent = body.get_string("parent", piscine->parent.c_str());
  }
  if (auth) {
    piscine->date = body.get_string("date", piscine->date.c_str());
  }
  PiscineDb::get().write();
  PiscineDb::unlock();
  return http::Response::ok();
}

http::Response post(http::Request r) {
  if (!is_authentified(r)) {
    return http::Response::unauthorized();
  }
  Json::Value o{Json::ValueType::objectValue};
  auto v = r.body_as_json();
  if (v == std::nullopt) return http::Response::bad_request();
  auto body = *v;
  if (!body.isObject()) return http::Response::bad_request();

  PiscineDb::lock();
  auto &piscine = PiscineDb::get().add_new();
  o["id"] = piscine.id;
  o["date"] = piscine.date = body.get_string("date");
  o["parent"] = piscine.parent = body.get_string("parent");
  PiscineDb::get().write();
  PiscineDb::unlock();
  return http::Response::Builder()
    .code(201)
    .json(o)
    .header("Location", (std::string("/api/piscine/") + o["id"].asString()).c_str())
    .close()
    .build();
}

http::Response del(http::Request r) {
  if (!is_authentified(r)) {
    return http::Response::unauthorized();
  }
  PiscineDb::lock();
  bool ok = PiscineDb::get().del_id(r.int_param("id", -1));
  PiscineDb::get().write();
  PiscineDb::unlock();
  if (!ok) return http::Response::not_found();
  return http::Response::ok();
}

} // namespace api::piscine
