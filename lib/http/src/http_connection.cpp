#include <connection.hpp>

#include <algorithm>
#include <arpa/inet.h>
#include <request.hpp>
#include <string>
#include <sys/logger.hpp>
#include <sys/socket.h>
#include <tcp_server.hpp>
#include <thread>
#include <unistd.h>

namespace http {

static std::string process_request(Connection &self, Request &r) {
  i64 bytes_sent;

  auto response = reinterpret_cast<HttpServer *>(self.m_server)
                      ->m_router.process_request(r);
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

static void close_connection(TcpServer *self, Connection *con) {
  self->m_connections_mutex.lock();
  auto it = std::find(self->m_active_connections.begin(),
                      self->m_active_connections.end(), con);
  self->m_active_connections.erase(it);
  self->m_connections_mutex.unlock();
  con->m_thread.detach();
  delete con;
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
    }

    buffer[bytes_received] = 0;
    // TODO: maybe not enough bytes in buffer when parsing request ?
    // => should read more.

    Request r = Request::parse(buffer);

    std::string res = process_request(*self, r);
    sys::info("%s:%-5d %s\t=> %s", inet_ntoa(self->m_socket_address.sin_addr),
              ntohs(self->m_socket_address.sin_port), r.first_line().c_str(),
              res.c_str());
    // break;
  }

  close(self->m_socket);

  close_connection(self->m_server, self);
}

} // namespace http
