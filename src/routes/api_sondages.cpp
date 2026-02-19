#include "../db.hpp"
#include "http/response.hpp"
#include "routes.hpp"
#include <json/json.h>

http::Response api::get_sondages(http::Request r) {
  // TODO: can't change eleve if not authentified ?
  Json::Value out{Json::ValueType::arrayValue};
  ITER_DB(s, id, SondagesDb) {
    Json::Value o{Json::ValueType::objectValue};
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

http::Response api::get_sondage(http::Request r) {
  Json::Value out{Json::ValueType::objectValue};
  r.body_as_params();
  i32 idx = r.int_param("id");
  bool ok = true;
  SondagesDb::lock();
  if (SondagesDb::get().items.size() > u32(idx) && idx >= 0) {
    auto &item = SondagesDb::get().items[idx];
    out["active"] = item.active;
    out["name"] = item.name;
    out["desc"] = item.desc;
    out["route"] = item.route;
    out["btn_text"] = item.button_text;
  } else {
    ok = false;
  }
  SondagesDb::unlock();
  if (!ok) return http::Response::not_found();
  return http::Response::Builder()
    .code(200)
    .json(out)
    .close()
    .build();
}

http::Response api::put_sondage(http::Request r) {
  if (!is_authentified(r))
    return http::Response::unauthorized();
  r.body_as_params();

  i32 idx = r.int_param("id");
  SondagesDb::lock();
  SondagesDb::get().items[idx].active =
      r.int_param("active", SondagesDb::get().items[idx].active);
  SondagesDb::get().items[idx].name =
      r.string_param("name", SondagesDb::get().items[idx].name.c_str());
  SondagesDb::get().items[idx].button_text = r.string_param(
      "btn_text", SondagesDb::get().items[idx].button_text.c_str());
  SondagesDb::get().items[idx].desc =
      r.string_param("desc", SondagesDb::get().items[idx].desc.c_str());
  SondagesDb::get().items[idx].route =
      r.string_param("route", SondagesDb::get().items[idx].route.c_str());
  SondagesDb::get().write();
  SondagesDb::unlock();
  return http::Response::ok();
}
