#include "defines.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>

static i32 sock;

void hot_reload_socket_create() {
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  sock = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_family = AF_INET;
  addr.sin_port = htons(1234);
  if (connect(sock, (sockaddr *)&addr, sizeof(addr)) < 0) {
    printf("Could not connect to hot reload server\n");
    std::exit(EXIT_FAILURE);
  }
}

void hot_reload_socket_close() {
  close(sock);
  sock = 0;
}

void do_hot_reload() {
  std::cout << "[compile] => hot reload!\n";
  write(sock, "garbage", 1);
}
