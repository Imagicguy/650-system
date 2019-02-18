#include "potato.h"
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

int main(int argc, char *argv[]) {
  // argv[1] = machine_name;
  // argv[2] = port_num;
  int status;
  int socket_fd[3];
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *master_hostname = argv[1];
  const char *master_port = argv[2];

  if (argc < 2) {
    cout << "Syntax: client <hostname>\n" << endl;
    return 1;
  }

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status =
      getaddrinfo(master_hostname, master_port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << master_hostname << "," << master_port << ")" << endl;
    return -1;
  } // if

  socket_fd[0] = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                        host_info_list->ai_protocol);
  if (socket_fd[0] == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << master_hostname << "," << master_port << ")" << endl;
    return -1;
  } // if

  cout << "Connecting to " << master_hostname << " on port " << master_port
       << "..." << endl;

  status = connect(socket_fd[0], host_info_list->ai_addr,
                   host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << master_hostname << "," << master_port << ")" << endl;
    return -1;
  } else {
    cout << "connection succeed!" << endl;
  }
  // connection succeed with master
  char buffer[512];
  recv(socket_fd[0], buffer, 512, 0);
  const char *my_port = (char *)buffer;
  cout << "OH I KNOW MY PORT IS " << my_port << endl;

  // start second socket -- server

  struct addrinfo host_info_2;
  struct addrinfo *host_info_list_2;
  const char *hostname_2 = NULL;
  int port_num = atoi(my_port) + (atoi(my_port) % 10) * 7;
  const char *port_2 = to_string(port_num).c_str();
  memset(&host_info_2, 0, sizeof(host_info_2));

  host_info_2.ai_family = AF_UNSPEC;
  host_info_2.ai_socktype = SOCK_STREAM;
  host_info_2.ai_flags = AI_PASSIVE;

  status = getaddrinfo(hostname_2, port_2, &host_info_2, &host_info_list_2);
  if (status != 0) {
    cerr << "ERROR: cannot addrinfo, error number: " << errno << endl;
    cerr << "  (" << hostname_2 << "," << port_2 << ")" << endl;
    return -1;
  } // if
  cout << "1 " << hostname_2 << endl;
  cout << "2 " << port_2 << endl;
  socket_fd[1] =
      socket(host_info_list_2->ai_family, host_info_list_2->ai_socktype,
             host_info_list_2->ai_protocol);
  if (socket_fd[1] == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname_2 << "," << port_2 << ")" << endl;
    return -1;
  } // if
  cout << "before bind ,player has hostname " << hostname_2 << " and " << port_2
       << endl;
  int yes = 1;
  status =
      setsockopt(socket_fd[1], SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd[1], host_info_list_2->ai_addr,
                host_info_list_2->ai_addrlen);
  if (status == -1) {
    cerr << "ERROR: cannot bind, error number: " << errno << endl;
    cerr << "  (" << hostname_2 << "," << port_2 << ")" << endl;
    return -1;
  } // if

  status = listen(socket_fd[1], 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname_2 << "," << port_2 << ")" << endl;
    return -1;
  } // if

  cout << "Waiting for connection on port " << port_2 << endl;

  char buffer1[512];
  recv(socket_fd[0], buffer1, 512, 0);
  string neighborInfo(buffer1);
  size_t segment = neighborInfo.find(" ");
  const char *neigh_host = neighborInfo.substr(0, segment).c_str();
  const char *neigh_port = neighborInfo.substr(segment + 1).c_str();

  // start third socket -- client neighbor

  struct addrinfo host_info_3;
  struct addrinfo *host_info_list_3;
  const char *hostname_3 = neigh_host;
  // const char *port_3 = neigh_port;
  int port_numn = atoi(neigh_port) + (atoi(neigh_port) % 10) * 7;
  const char *port_3 = to_string(port_numn).c_str();
  cout << "Im gonna connect to " << port_3 << endl;

  memset(&host_info_3, 0, sizeof(host_info_3));
  host_info_3.ai_family = AF_UNSPEC;
  host_info_3.ai_socktype = SOCK_STREAM;

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

  freeaddrinfo(host_info_list);
  freeaddrinfo(host_info_list_2);
  freeaddrinfo(host_info_list_3);
  close(socket_fd[0]);
  close(socket_fd[1]);
  close(socket_fd[2]);

  return 0;
}
//先建立两个socket,再发送建立Neighbor请求，得到neighbor地址后connect
