#include "potato.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

int main(int argc, char *argv[]) {
  // argv[1] = machine_name;
  // argv[2] = port_num;
  int status;
  int socket_fd[4];
  // 0: socket to master
  // 1:player listen as server
  // 2:connect to left'server
  // 3:new socket created by accept for right
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *master_hostname = argv[1];
  const char *master_port = argv[2];

  if (argc < 2) {
    cout << "Syntax: client <hostname>\n" << endl;
    return 1;
  }

  // connection succeed with master
  char buffer[512];
  recv(socket_fd[0], buffer, 512, 0);
  string Info(buffer);
  size_t segment1 = Info.find(" ");

  const char *my_num = Info.substr(0, segment1).c_str();
  const char *total_num = Info.substr(segment1 + 1).c_str();

  cout << "Connected as player " << my_num << " out of " << total_num
       << " total players " << endl;

  //  send server info to master
  string str1(hostname_2);
  string str2(port_2);
  cout << "host is " << hostname_2 << endl;
  cout << "Waiting for connection on port " << port_2 << endl;
  string my_server_info = str1 + " " + str2;
  const char *msgs = my_server_info.c_str();

  send(socket_fd[0], msgs, 512, 0);
  // cout << "here?" << endl;
  cout << "host is " << hostname_2 << endl;
  cout << "Waiting for connection on port " << port_2 << endl;
  // get neighbor info from master
  char buffer1[512];
  recv(socket_fd[0], buffer1, 512, 0);
  string neighborInfo(buffer1);
  size_t segment = neighborInfo.find(" ");
  const char *hostname_3 = neighborInfo.substr(0, segment).c_str();
  const char *port_3 = neighborInfo.substr(segment + 1).c_str();

  // start third socket -- client neighbor
  struct addrinfo host_info_3;
  struct addrinfo *host_info_list_3;
  cout << "Im gonna connect to " << hostname_3 << " " << port_3 << endl;

  memset(&host_info_3, 0, sizeof(host_info_3));
  host_info_3.ai_family = AF_UNSPEC;
  host_info_3.ai_socktype = SOCK_STREAM;
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(struct sockaddr_storage);

  socket_fd[3] =
      accept(socket_fd[1], (struct sockaddr *)&socket_addr, &socket_addr_len);

  if (socket_fd[3] == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    return -1;
  }

  status = getaddrinfo(hostname_3, port_3, &host_info_3, &host_info_list_3);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname_3 << "," << port_3 << ")" << endl;
    return -1;
  } // if

  socket_fd[2] =
      socket(host_info_list_3->ai_family, host_info_list_3->ai_socktype,
             host_info_list_3->ai_protocol);
  if (socket_fd[2] == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname_3 << "," << port_3 << ")" << endl;
    return -1;
  } // if

  cout << "Connecting to " << hostname_3 << " on port " << port_3 << "..."
       << endl;

  status = connect(socket_fd[2], host_info_list_3->ai_addr,
                   host_info_list_3->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket " << errno << endl;
    cerr << "  (" << hostname_3 << "," << port_3 << ")" << endl;
    return -1;
  } else {
    cout << "connection succeed!" << endl;
  }

  /*if (atoi(my_num) == 1) {
    char bufferInfo[512];
    recv(socket_fd[2], bufferInfo, 512, 0);
    socket_fd[3] =
        accept(socket_fd[1], (struct sockaddr *)&socket_addr, &socket_addr_len);

    if (socket_fd[3] == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      return -1;
    }
    }*/
  char msg[] = "I'm ready";
  if (send(socket_fd[0], msg, sizeof(msg), 0) == -1) {
    perror("ERROR: IM READY");
  }
  // 0: socket to master
  // 1:player listen as server
  // 2:connect to left'server
  // 3:new socket created by accept for right
  vector<int> socket_list;
  socket_list.push_back(socket_fd[0]);
  socket_list.push_back(socket_fd[2]);
  socket_list.push_back(socket_fd[3]);
  socket_list.push_back(socket_fd[1]);
  cout << my_num << " ready to play" << endl;
  Game game(socket_list);
  game.pass(atoi(my_num));
  freeaddrinfo(host_info_list);
  freeaddrinfo(host_info_list_2);
  freeaddrinfo(host_info_list_3);
  close(socket_fd[0]);
  close(socket_fd[1]);
  close(socket_fd[2]);

  return 0;
}
//先建立两个socket,再发送建立Neighbor请求，得到neighbor地址后connect
