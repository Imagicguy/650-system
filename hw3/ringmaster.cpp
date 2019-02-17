#include "potato.h"
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

in_port_t get_in_port(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return (((struct sockaddr_in *)sa)->sin_port);
  }

  return (((struct sockaddr_in6 *)sa)->sin6_port);
}

int main(int argc, char *argv[]) {
  // argv[1] = port_num
  // argv[2] = num_players
  // argv[3] = num_hops
  if (argc != 4) {
    cout << "invalid input!" << endl;
    return -1;
  }

  int num_players = atoi(argv[2]);
  int num_hops = atoi(argv[1]);
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  const char *port = argv[1];

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

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

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } // if

  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } // if

  cout << "Waiting for connection on port " << port << endl;
  struct sockaddr_storage socket_addr[num_players];
  socklen_t socket_addr_len = sizeof(struct sockaddr_storage);
  int sockets_fd[num_players];
  int curr = 0;

  while (curr < num_players) {
    sockets_fd[curr] = accept(socket_fd, (struct sockaddr *)&socket_addr[curr],
                              &socket_addr_len);

    if (sockets_fd[curr] == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      return -1;
    }
    in_port_t port_num = get_in_port((struct sockaddr *)&socket_addr[curr]);
    void *msg = &port_num;
    send(sockets_fd[curr], msg, 9, 0);
    cout << "Player " << curr << " is ready to play" << endl;
    curr++;
  }
  cout << "All player are ready!" << endl;

  return 0;
}
