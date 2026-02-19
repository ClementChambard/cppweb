#include "../db.hpp"
#include "routes.hpp"
#include <algorithm>

http::Response api::get_piscine(http::Request r) {
  Json::Value out{Json::ValueType::arrayValue};
  ITER_DB(s, id, PiscineDb) {
    Json::Value o{Json::ValueType::objectValue};
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

http::Response api::piscine_id(http::Request r) {
  i32 idx = r.int_param("id");

  // AUTH check
  if (!is_authentified(r)) {
    if (r.params.contains("date"))
      return http::Response::unauthorized();
    
    PiscineDb::lock();
    bool parend_set = PiscineDb::get().items[idx].parent != "";
    PiscineDb::unlock();
    if (r.params.contains("parent") && parend_set)
      return http::Response::unauthorized();
  }

  r.body_as_params();

  PiscineDb::lock();

  auto old_date = PiscineDb::get().items[idx].date;
  PiscineDb::get().items[idx].parent = r.string_param("parent", PiscineDb::get().items[idx].parent.c_str());
  PiscineDb::get().items[idx].date = r.string_param("date", PiscineDb::get().items[idx].date.c_str());
  if (old_date != PiscineDb::get().items[idx].date)
    std::sort(PiscineDb::get().items.begin(), PiscineDb::get().items.end(), [](auto a, auto b) -> bool { return a.date < b.date; });
  PiscineDb::get().write();

  PiscineDb::unlock();

  return http::Response::Builder()
      .code(303)
      .header("location", r.header("Referer").c_str())
      .close()
      .build();
}

http::Response api::piscine(http::Request r) {
  // AUTH check
  if (!is_authentified(r)) {
    return http::Response::unauthorized();
  }

  // TODO: if someone access to an element while a deletion is occuring, this
  // may cause problems.
  //       solution: use unique identifier for each rdv ?
  r.body_as_params();
  auto action = r.string_param("action");
  if (action == "delete") {
    i32 id = r.int_param("id", -1);

    PiscineDb::lock();
    auto size = PiscineDb::get().items.size();
    PiscineDb::unlock();

    if (id < 0 || u32(id) >= size) {
      return http::Response::bad_request();
    }

    PiscineDb::lock();
    PiscineDb::get().items.erase(PiscineDb::get().items.begin() + id);
    PiscineDb::get().write();
    PiscineDb::unlock();
  } else if (action == "create") {
    Piscine piscine;
    piscine.parent = r.string_param("parent");
    piscine.date = r.string_param("date");
    PiscineDb::lock();
    PiscineDb::get().items.push_back(piscine);
    std::sort(PiscineDb::get().items.begin(), PiscineDb::get().items.end(),
              [](auto a, auto b) -> bool { return a.date < b.date; });
    // TODO: insert sorted ?
    PiscineDb::get().write();
    PiscineDb::unlock();
  } else {
    return http::Response::bad_request();
  }
  return http::Response::Builder()
      .code(303)
      .header("location", r.header("Referer").c_str())
      .close()
      .build();
}
