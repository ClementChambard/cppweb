#include "ws_connection.hpp"
#include "http_server.hpp"
#include <sstream>
#include <string>
#include <sys/logger.hpp>
#include <sys/sha1.hpp>
#include <vector>

HttpServer *WS_SERVER;

static constexpr char const *WS_ENDPOINT = "/dev/ws/hotreload";

void ws_open_connection(HttpServer *serv) { WS_SERVER = serv; }

void ws_make_route(struct Router &r) { r.wo_ws(WS_ENDPOINT); }

void ws_send_json(Json::Value v) {
  std::ostringstream oss;
  auto w = Json::StreamWriterBuilder().newStreamWriter();
  w->write(v, &oss);
  ws_send_string(oss.str());
}

void ws_send_string(const std::string &s) {
  std::vector<u8> INPUT = {(u8 *)s.data(), (u8 *)s.data() + s.size()};

  sys::log_extra("WS", s.c_str());

  WS_SERVER->ws_broadcast_message(WS_ENDPOINT, INPUT);
}
