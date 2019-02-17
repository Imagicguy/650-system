#ifndef __POTATO_H__
#define __POTATO_H__
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;
int serverSocket(const char *hostname, const char *port) {

  int status = 0;
  int socket_fd = 0;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;
  cout << hostname << endl;
  cout << port << endl;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } // if

  socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } // if
  return socket_fd;
}
int clientSocket(char *hostname, char *port, struct addrinfo *host_info,
                 struct addrinfo *host_info_list) {
  int status = 0;
  int socket_fd = 0;
  host_info_list = NULL;

  status = getaddrinfo(hostname, port, host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } // if

  socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } // if

  cout << "Connecting to " << hostname << " on port " << port << "..." << endl;
  return socket_fd;
}
class player {};

#endif
