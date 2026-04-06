#include "ws_connection.hpp"
#include <http/tcp_server.hpp>
#include <http/ws.hpp>
#include <json/json.h>
#include <sstream>
#include <thread>

http::TcpServer *WS_SERVER;

void ws_add_script_to_document(html::Document &doc) {
  auto &head = std::get<html::Element>(doc.root_node.children[0]);
  head.children.push_back(html::html("script", {{"src", "/test.js"}}, {}, {}));
}

void ws_open_connection() {
  WS_SERVER = new http::WsServer("0.0.0.0", 8081);

  WS_SERVER->start_listen(true);
}

void ws_close_connection() {
  ws_send_string("{\"action\":\"close\"}");
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  WS_SERVER->stop();
}

void ws_send_json(Json::Value v) {
  std::ostringstream oss;
  auto w = Json::StreamWriterBuilder().newStreamWriter();
  w->write(v, &oss);
  ws_send_string(oss.str());
}

void ws_send_string(const std::string &s) {
  std::vector<u8> INPUT = {(u8 *)s.data(), (u8 *)s.data() + s.size()};

  auto DATA = write_ws(INPUT);

  WS_SERVER->broadcast_message(DATA);
}
