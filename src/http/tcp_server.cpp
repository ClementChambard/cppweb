#include "tcp_server.hpp"
#include "html/code_instance.hpp"
#include "http/connection.hpp"

#include <arpa/inet.h>
#include <html/code_builder.hpp>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/logger.hpp>
#include <sys/read_file.hpp>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

struct ServerCommand {
  std::string cmd;
  void parse();
};

void ServerCommand::parse() {}

static bool execute_server_command(http::TcpServer& serv, ServerCommand const &cmd) {
  if (cmd.cmd == "/stop") {
    return true;
  }
  if (cmd.cmd == "/reload_html") {
    html::cleanup_cache();
    sys::info("[/reload_html]: Successfully reloaded html");
  }
  if (cmd.cmd == "/help") {
    sys::info("[/help]: Available commands:\n"
        " - /stop: stops the server\n"
        " - /help: show this message\n"
        " - /reload_html: reloads the in-memory html\n");
  }
  return false;
}

namespace http {

TcpServer::TcpServer(char const *ip_address, i32 port)
    : m_ip_address(ip_address), m_port(port), m_socket(),
      m_socket_address(), m_should_quit(false) {
  m_socket_address.sin_family = AF_INET;
  m_socket_address.sin_port = htons(m_port);
  m_socket_address.sin_addr.s_addr = inet_addr(ip_address);

  m_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (m_socket < 0) {
    // TOOD: log
    sys::fatal_error("Cannot create socket");
  }

  if (bind(m_socket, (sockaddr *)&m_socket_address,
           sizeof(m_socket_address)) < 0) {
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
    client->m_socket = accept(self->m_socket, (sockaddr *)&client->m_socket_address, &client_addr_size);
    if (client->m_socket < 0) {
      if (self->m_should_quit) break;
      sys::error("Accept failed");
      continue;
    }
    self->m_connections_mutex.lock();
    self->m_active_connections.push_back(client);
    client->start_thread();
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

void TcpServer::start_listen() {
  if (listen(m_socket, 20) < 0) {
    sys::fatal_error("Socket listen failed");
  }

  sys::info("*** Listening on ADDRESS: %s PORT: %d ***",
            m_ip_address.c_str(), m_port);

  m_server_thread = std::thread(server_thread_run, this);

  server_console_loop(*this);
}

} // namespace http
