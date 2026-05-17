#include "connection.hpp"
#include "http_server.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <http/request.hpp>
#include <string>
#include <sys/logger.hpp>
#include <thread>
#include <unistd.h>

static std::string process_request(Connection &self, http::Request &r) {
  i64 bytes_sent;

  http::Response response;
  if (auto *ws = self.m_server->m_router.maybe_process_ws(r); ws != nullptr) {
    self.ws = ws;
    ws->fd = self.m_socket;
    response = http::WSConnection::respond_to_ws(r);
  } else {
    response = self.m_server->m_router.process_request(r);
  }
  std::vector<u8> server_message = std::vector<u8>(response);

  bytes_sent =
      write(self.m_socket, server_message.data(), server_message.size());

  if (u64(bytes_sent) != server_message.size()) {
    sys::error("%s:%-5d => Error sending response to client",
               inet_ntoa(self.m_socket_address.sin_addr),
               ntohs(self.m_socket_address.sin_port));
  }
  return response.first_line();
}

void http_client_thread(Connection *self) {
  static constexpr u64 BUFFER_SIZE = 30720;

  i32 bytes_received;

  char buffer[BUFFER_SIZE + 1];

  while (true) {
    buffer[0] = 0;
    bytes_received = read(self->m_socket, buffer, BUFFER_SIZE);

    if (bytes_received == 0) {
      break;
    } else if (bytes_received < 0) {
      sys::error(
          "%s:%-5d => Failed to read bytes from client socket connection",
          inet_ntoa(self->m_socket_address.sin_addr),
          ntohs(self->m_socket_address.sin_port));
      break;
    }

    if (bytes_received == BUFFER_SIZE) {
      sys::warn(" * Request might be too long for internal buffer...");
      // TODO: maybe not enough bytes in buffer when parsing request ?
      // => should read more.
    }

    buffer[bytes_received] = 0;

    http::Request r = http::Request::parse(buffer);

    std::string res = process_request(*self, r);
    sys::info("%s:%-5d %s\t=> %s", inet_ntoa(self->m_socket_address.sin_addr),
              ntohs(self->m_socket_address.sin_port), r.first_line().c_str(),
              res.c_str());

    if (self->ws) {
      sys::info("%s:%-5d => New WS Connection",
                inet_ntoa(self->m_socket_address.sin_addr),
                ntohs(self->m_socket_address.sin_port));
      self->ws->run();
      sys::info("%s:%-5d => WS Connection Closed",
                inet_ntoa(self->m_socket_address.sin_addr),
                ntohs(self->m_socket_address.sin_port));
      delete self->ws;
      break;
    }
  }

  close(self->m_socket);

  self->m_server->m_connections_mutex.lock();
  auto it = std::find(self->m_server->m_active_connections.begin(),
                      self->m_server->m_active_connections.end(), self);
  self->m_server->m_active_connections.erase(it);
  self->m_server->m_connections_mutex.unlock();
  std::thread this_thread = std::move(self->m_thread);
  delete self;
  this_thread.detach();
}
