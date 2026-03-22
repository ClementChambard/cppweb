#include "connection.hpp"

#include "request.hpp"
#include "response.hpp"
#include "tcp_server.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include "ws.hpp"

#include <sys/sha1.hpp>

namespace http {

static void process_request(Connection &self, Request &r) {
  i64 bytes_sent;

  auto key = r.header("Sec-WebSocket-Key");

  key = base_64_sha1(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

  auto response = Response::Builder()
                      .code(101)
                      .header("Upgrade", "WebSocket")
                      .header("Connection", "Upgrade")
                      .header("Sec-WebSocket-Accept", key.c_str())
                      .build(true);

  std::string server_message = std::string(response);

  bytes_sent =
      write(self.m_socket, server_message.c_str(), server_message.size());

  (void)bytes_sent;
}

static bool process_message(Connection &self, std::vector<u8> const &message) {
  FrameHeader *h = (FrameHeader *)message.data();

  if (!h->MASK || h->OPCODE == 8) {
    return false;
  }

  std::vector<u8> DATA = get_ws(*h);

  std::cout << std::string((char *)DATA.data(), DATA.size()) << "\n";

  std::vector<u8> OUT{(u8 *)"ok", (u8 *)"ok" + 2};

  std::vector<u8> response = write_ws(OUT);

  write(self.m_socket, response.data(), response.size());

  return true;
}

static void close_connection(TcpServer *self, Connection *con) {
  self->m_connections_mutex.lock();
  auto it = std::find(self->m_active_connections.begin(),
                      self->m_active_connections.end(), con);
  self->m_active_connections.erase(it);
  self->m_connections_mutex.unlock();
  con->m_thread.detach();
  delete con;
}

void ws_client_thread(Connection *self) {
  static constexpr u64 BUFFER_SIZE = 30720;

  i32 bytes_received;

  char buffer[BUFFER_SIZE + 1];

  buffer[0] = 0;
  bytes_received = read(self->m_socket, buffer, BUFFER_SIZE);
  Request r = Request::parse(buffer);
  process_request(*self, r);

  while (true) {
    buffer[0] = 0;
    bytes_received = read(self->m_socket, buffer, BUFFER_SIZE);

    if (bytes_received == 0) {
      break;
    } else if (bytes_received < 0) {
      break;
    }

    // TODO: maybe not enough bytes in buffer when parsing request ?
    // => should read more.

    if (!process_message(*self, std::vector<u8>{(u8 *)buffer,
                                                (u8 *)buffer + bytes_received}))
      break;
  }

  std::cout << "CLOSE!!!\n";

  close(self->m_socket);

  close_connection(self->m_server, self);
}

} // namespace http
