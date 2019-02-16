#include <cstring>
#include <iostream>
#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

int main(int argc, char *argv[]) {
  // argv[1] = port_num
  // argv[2] = num_players
  // argv[3] = num_hops
  if (argc != 3) {
    cout << "invalid input!" << endl;
    return -1;
  }
  int port_num = *argv[1];
  int num_players = *argv[2];
  int num_hops = *argv[3];
  const char *hostname = NULL;
  const char *port = "4444";
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;

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
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd[num_players];
  int currPlayer = 0;
  while (currPlayer < num_players) {
    client_connection_fd[currPlayer] =
        accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    cout << "I got " << currPlayer << " players" << endl;
  }

  return 0;
}
