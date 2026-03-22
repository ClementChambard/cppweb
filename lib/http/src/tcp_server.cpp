#include <connection.hpp>
#include <tcp_server.hpp>

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/logger.hpp>
#include <sys/read_file.hpp>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include "ws.hpp"

struct ServerCommand {
  std::string cmd;
  void parse();
};

void ServerCommand::parse() {}

static bool execute_server_command(http::TcpServer &serv,
                                   ServerCommand const &cmd) {
  (void)serv;
  if (cmd.cmd == "/stop") {
    return true;
  }
  if (cmd.cmd == "/help") {
    sys::info("[/help]: Available commands:\n"
              " - /stop: stops the server\n"
              " - /help: show this message\n");
    return false;
  }

  std::vector<u8> OUT{(u8 *)cmd.cmd.data(), (u8 *)cmd.cmd.data() + cmd.cmd.size()};
  auto out = write_ws(OUT);

  serv.broadcast_message(out);
  return false;
}

namespace http {

TcpServer::TcpServer(char const *ip_address, i32 port, fp_connection *fn)
    : m_connection_func(fn), m_ip_address(ip_address), m_port(port), m_socket(),
      m_socket_address(), m_should_quit(false) {
  m_socket_address.sin_family = AF_INET;
  m_socket_address.sin_port = htons(m_port);
  m_socket_address.sin_addr.s_addr = inet_addr(ip_address);

  m_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (m_socket < 0) {
    // TOOD: log
    sys::fatal_error("Cannot create socket");
  }

  if (bind(m_socket, (sockaddr *)&m_socket_address, sizeof(m_socket_address)) <
      0) {
    sys::fatal_error("Cannot connect socket to address");
  }
}

TcpServer::~TcpServer() {
  m_should_quit = true;
  shutdown(m_socket, SHUT_RDWR); // this does not shutdown the socket ????
  close(m_socket);
  m_server_thread.join();
  m_connections_mutex.lock();
  for (auto const &client : m_active_connections) {
    delete client;
  }
  m_connections_mutex.unlock();
  sys::info("Server stopped");
}

void server_thread_run(TcpServer *self) {
  socklen_t client_addr_size = sizeof(sockaddr_in);

  Connection *client = new Connection;
  while (true) {
    client->m_server = self;
    client->m_socket =
        accept(self->m_socket, (sockaddr *)&client->m_socket_address,
               &client_addr_size);
    if (client->m_socket < 0) {
      if (self->m_should_quit)
        break;
      sys::error("Accept failed");
      continue;
    }
    self->m_connections_mutex.lock();
    self->m_active_connections.push_back(client);
    client->start_thread(self->m_connection_func);
    self->m_connections_mutex.unlock();
    client = new Connection;
  }
  delete client;
}

void server_console_loop(TcpServer &self) {
  while (true) {
    ServerCommand cmd;
    std::getline(std::cin, cmd.cmd, '\n');
    cmd.parse();
    if (execute_server_command(self, cmd)) {
      break;
    }
  }
}

void TcpServer::start_listen(bool async) {
  if (listen(m_socket, 20) < 0) {
    sys::fatal_error("Socket listen failed");
  }

  sys::info("*** Listening on ADDRESS: %s PORT: %d ***", m_ip_address.c_str(),
            m_port);

  m_server_thread = std::thread(server_thread_run, this);

  if (!async) {
    server_console_loop(*this);
  }
}

void TcpServer::broadcast_message(std::vector<u8> const &message_bytes) {
  for (auto c : m_active_connections) {
    write(c->m_socket, message_bytes.data(), message_bytes.size());
  }
}

} // namespace http
