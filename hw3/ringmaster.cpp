#include "potato.h"
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
using namespace std;

in_port_t get_in_port(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return (((struct sockaddr_in *)sa)->sin_port);
  }

  return (((struct sockaddr_in6 *)sa)->sin6_port);
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    cout << "invalid input!" << endl;
    return -1;
  }

  // int num_players = atoi(argv[2]);
  int num_players = atoi(argv[2]);
  int num_hops = atoi(argv[3]);
  int status;
  int socket_fd;

  cout << "Potato Ringmaster" << endl;
  cout << "Player = " << num_players << endl;
  cout << "Hops = " << num_hops << endl;
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

  int yes = 0;
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

  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(struct sockaddr_storage);
  int sockets_fd[num_players + 1];
  int curr = 0;
  char hostArr[NI_MAXHOST];
  char portArr[NI_MAXSERV];
  string playerInfo;

  vector<string> serverInfo;
  while (curr < num_players) {
    sockets_fd[curr] =
        accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);

    if (sockets_fd[curr] == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      return -1;
    }
    cout << "Player " << curr << " is ready to play" << endl;
    getnameinfo((struct sockaddr *)&socket_addr, socket_addr_len, hostArr,
                sizeof(hostArr), portArr, sizeof(portArr), 0);

    playerInfo = to_string(curr) + " " + to_string(num_players);
    const char *msg = playerInfo.c_str();
    send(sockets_fd[curr], msg, 512, 0);
    char bufferInfo[512];
    recv(sockets_fd[curr], bufferInfo, 512, 0);
    serverInfo.push_back(bufferInfo);
    curr++;
  }
  sockets_fd[num_players] = sockets_fd[0];
  for (int i = 0; i < num_players; i++) {
    string info = serverInfo.at(i);
    const char *msg = info.c_str();
    send(sockets_fd[i + 1], msg, 512, 0);
  }

  for (int i = 0; i < num_players; i++) {
    string mstr("it's your turn");

    send(sockets_fd[i + 1], mstr.c_str(), 512, 0);
  }

  char msg3[] = "I'm ready";
  for (int i = 0; i < num_players; i++) {
    if (send(sockets_fd[i], msg3, 512, 0) == -1) {
      perror("ERROR: 23");
    }
  }

  Game game(sockets_fd, num_players, num_hops);
  potato hot_potato;
  hot_potato.is_cold = 0;
  hot_potato.remain_hop = num_hops;
  hot_potato.total_hop = num_hops;

  srand((unsigned int)time(NULL));
  int first = rand() % num_players;
  cout << "Ready to start game,sending potato to player " << first << endl;
  if (send(sockets_fd[first], &hot_potato, sizeof(potato), 0) == -1) {
    perror("ERROR:SEND FIRST FAILED!");
  }

  if (num_hops > 0) {
    game.start(num_hops);
  }
  game.over();

  return 0;
}
