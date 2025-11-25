#include "connection.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <http/request.hpp>
#include <http/tcp_server.hpp>
#include <string>
#include <sys/logger.hpp>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace http {

void process_request(Connection &self, Request &r) {
  i64 bytes_sent;

  auto response = self.m_server->m_router.process_request(r);
  std::string server_message = std::string(response);

  bytes_sent =
      write(self.m_socket, server_message.c_str(), server_message.size());

  if (u64(bytes_sent) == server_message.size()) {
    sys::info(" ==> %s", response.first_line().c_str());
  } else {
    sys::error(" ==> Error sending response to client\n");
  }
}

void close_connection(TcpServer *self, Connection *con) {
  self->m_connections_mutex.lock();
  auto it = std::find(self->m_active_connections.begin(),
                      self->m_active_connections.end(), con);
  self->m_active_connections.erase(it);
  self->m_connections_mutex.unlock();
  con->m_thread.detach();
  sys::info("------ connection closed ------");
  delete con;
}

void client_thread(Connection *self) {
  static constexpr u64 BUFFER_SIZE = 30720;

  sys::info("------ new connection from ADDRESS: %s PORT: %d ------",
            inet_ntoa(self->m_socket_address.sin_addr),
            ntohs(self->m_socket_address.sin_port));

  i32 bytes_received;

  char buffer[BUFFER_SIZE];

  while (true) {
    buffer[0] = 0;
    bytes_received = read(self->m_socket, buffer, BUFFER_SIZE);

    if (bytes_received == 0) {
      break;
    } else if (bytes_received < 0) {
      sys::error(" ==> Failed to read bytes from client socket connection");
      break;
    }

    Request r = Request::parse(buffer);
    sys::info(" ==> %s", r.first_line().c_str());

    process_request(*self, r);
    // break;
  }

  close(self->m_socket);

  close_connection(self->m_server, self);
}

void Connection::start_thread() { m_thread = std::thread(client_thread, this); }

} // namespace http
