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

int set_client(const char *hostname, const char *port) {
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *master_hostname = hostname;
  const char *master_port = port;

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(master_hostname, master_port, &host_info, &host_info_list) !=
      0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << master_hostname << "," << master_port << ")" << endl;
    return -1;
  }
  int socket_fd;
  if ((socket_fd =
           socket(host_info_list->ai_family, host_info_list->ai_socktype,
                  host_info_list->ai_protocol)) == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << master_hostname << "," << master_port << ")" << endl;
    return -1;
  }

  cout << "Connecting to " << master_hostname << " on port " << master_port
       << "..." << endl;

  if (connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen) ==
      -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << master_hostname << "," << master_port << ")" << endl;
    return -1;
  } else {
    cout << "connection succeed!" << endl;
  }
  return socket_fd;
}

int set_server(string &hostname_2, string &port_2) {
  // start second socket -- server
  struct addrinfo host_info_2;
  struct addrinfo *host_info_list_2;
  char hostname_mch[NI_MAXHOST];
  struct hostent *aka;
  int status;
  int socket_fd;
  if (gethostname(hostname_mch, NI_MAXHOST) == -1) {
    perror("gethostname");
    exit(1);
  };
  aka = gethostbyname(hostname_mch);
  if (aka == NULL) {
    perror("gethostbyname");
    exit(1);
  }
  cout << "hostname is " << hostname_2 << endl;
  hostname_2 = inet_ntoa(*((struct in_addr *)aka->h_addr_list[0]));
  int port2 = 20000;
  memset(&host_info_2, 0, sizeof(host_info_2));

  host_info_2.ai_family = AF_UNSPEC;
  host_info_2.ai_socktype = SOCK_STREAM;
  host_info_2.ai_flags = AI_PASSIVE;

  while (true) {
    status = getaddrinfo(hostname_2.c_str(), to_string(port2).c_str(),
                         &host_info_2, &host_info_list_2);
    if (status != 0) {
      cerr << "ERROR: cannot addrinfo, error number: " << errno << endl;
      cerr << "  (" << hostname_2 << "," << to_string(port2).c_str() << ")"
           << endl;
      return -1;
    } // if

    socket_fd =
        socket(host_info_list_2->ai_family, host_info_list_2->ai_socktype,
               host_info_list_2->ai_protocol);
    if (socket_fd < 0) {
      cerr << "Error: cannot create socket" << endl;
      cerr << "  (" << hostname_2 << "," << to_string(port2).c_str() << ")"
           << endl;
      return -1;
    } // if
    int yes = 0;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list_2->ai_addr,
                  host_info_list_2->ai_addrlen);
    if (status == -1) {
      // cerr << "ERROR: cannot bind, error number: " << endl;
      port2++;
    } else {
      cout << "bind succeed,port is " << to_string(port2).c_str() << endl;
      break;
    }
  }
  cout << "after while fd is " << socket_fd << endl;
  port_2 = (char *)to_string(port2).c_str();
  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname_2 << "," << port_2 << ")" << endl;
    return -1;
  } // if

  cout << "host is " << hostname_2 << endl;
  cout << "Waiting for connection on port " << port_2 << endl;
  return socket_fd;
}
int main(int argc, char *argv[]) {
  // int status;
  int socket_fd[4];
  // 0: socket to master
  // 1:player accepter
  // 2:connect to left'server
  // 3:new socket created by accept for right
  if (argc < 2) {
    cout << "Syntax: client <hostname>\n" << endl;
    return 1;
  }

  socket_fd[0] = set_client(argv[1], argv[2]);
  if (socket_fd[0] == 0)
    perror("ERROR: FAILED ON FD[0]");
  string my_host_s = "";
  string my_port_s = "";
  socket_fd[1] = set_server(my_host_s, my_port_s);
  cout << "out of function fd is  " << socket_fd[1] << endl;
  if (socket_fd[1] < 0)
    perror("ERROR: FAILED ON FD[1]");
  const char *my_host = my_host_s.c_str();
  const char *my_port = my_port_s.c_str();

  cout << "my_host is " << my_host << endl;
  cout << "my_port is " << my_port << endl;

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
  string str1(my_host);
  string str2(my_port);
  cout << "my host is " << my_host << endl;
  cout << "Im waiting for connection on port " << my_port << endl;
  string my_server_info = str1 + " " + str2;
  const char *msgs = my_server_info.c_str();

  send(socket_fd[0], msgs, 512, 0);

  // get neighbor info from master
  char buffer1[512];
  recv(socket_fd[0], buffer1, 512, 0);

  string neighborInfo(buffer1);
  size_t segment = neighborInfo.find(" ");
  const char *hostname_3 = neighborInfo.substr(0, segment).c_str();
  const char *port_3 = neighborInfo.substr(segment + 1).c_str();
  cout << "connect to neighbor " << hostname_3 << " " << port_3 << endl;
  socket_fd[2] = set_client(hostname_3, port_3);
  if (socket_fd[2] == 0)
    perror("ERROR: FAILED ON FD[2]");

  char buffer3[512];
  recv(socket_fd[0], buffer3, 512, 0);
  cout << "my turn? " << buffer3 << endl;

  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(struct sockaddr_storage);

  socket_fd[3] =
      accept(socket_fd[1], (struct sockaddr *)&socket_addr, &socket_addr_len);
  cout << "fd[1] is " << socket_fd[1] << endl;
  cout << "fd[2] is " << socket_fd[2] << endl;
  cout << "fd[3] is " << socket_fd[3] << endl;
  if (socket_fd[3] == -1) {
    cerr << "ERROR: cannot accept connection , error number: " << errno << endl;
    return -1;
  } else {
    cout << "Got!" << endl;
  }

  vector<int> socket_list;

  // 0: socket to master
  // 1:player listen as server
  // 2:connect to left'server
  // 3:new socket created by accept for right

  /*send(socket_fd[2], msge, 512, 0);
    send(socket_fd[3], msge, 512, 0);*/
  char buffer4[512];
  int bytes = recv(socket_fd[0], buffer4, 512, MSG_WAITALL);
  cout << "buffer size is " << bytes << endl;
  /*char buffer5[512];
  recv(socket_fd[3], buffer5, 512, 0);
  cout << "buffer is " << buffer5 << endl;*/
  socket_list.push_back(socket_fd[0]);
  socket_list.push_back(socket_fd[2]);
  socket_list.push_back(socket_fd[3]);
  socket_list.push_back(socket_fd[1]);
  cout << my_num << " ready to play" << endl;
  Game game(socket_list);
  game.pass(atoi(my_num));

  return 0;
}
