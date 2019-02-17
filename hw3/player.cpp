#include "potato.h"
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

int main(int argc, char *argv[]) {
  // argv[1] = machine_name;
  // argv[2] = port_num;
  int status[3];
  status[0] = 1;
  int socket_fd[3];
  struct addrinfo host_info_1;
  struct addrinfo *host_info_list[3];
  char *hostname[3];
  hostname[0] = argv[1];
  char *port[3];
  hostname[0] = argv[2];

  if (argc < 2) {
    cout << "Syntax: client <hostname>\n" << endl;
    return 1;
  }

  memset(&host_info_1, 0, sizeof(struct addrinfo));
  host_info_1.ai_family = AF_UNSPEC;
  host_info_1.ai_socktype = SOCK_STREAM;
  cout << "reach ehre1" << endl;
  socket_fd[0] =
      clientSocket(hostname[0], port[0], &host_info_1, host_info_list[0]);

  status[0] = connect(socket_fd[0], host_info_list[0]->ai_addr,
                      host_info_list[0]->ai_addrlen);
  cout << "reach here?" << endl;
  if (status[0] == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << hostname[0] << "," << port[0] << ")" << endl;
    return -1;
  } else {
    cout << "connection succeed!" << endl;
  }
  // connection succeed with master
  void *buffer;
  recv(socket_fd[0], buffer, 9, 0);
  int *port_spc = (int *)buffer;
  cout << "OH I KNOW MY PORT IS " << *port_spc << endl;
  int a = 3;
  std::string port_serv;
  std::stringstream out;
  out << *port_spc;
  port_serv = out.str();

  freeaddrinfo(host_info_list[0]);
  close(socket_fd[0]);

  return 0;
}
//先建立两个socket,再发送建立Neighbor请求，得到neighbor地址后connect
