#pragma once

#include "request.hpp"
#include "response.hpp"
#include <defines.hpp>
#include <string>

namespace http {

struct WSConnection {
  using MsgCallback = void(WSConnection &con, u8 const *data, u64 size);

  i32 fd = -1;
  std::string origin_endpoint;
  MsgCallback *on_message = nullptr;

  void send_message(std::string const &msg) {
    send_message((u8 const *)msg.data(), msg.size());
  }
  void send_message(u8 const *data, u64 length);

  void run();

  static Response respond_to_ws(Request const &r);
};

} // namespace http
