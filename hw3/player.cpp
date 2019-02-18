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

  // start second socket -- server
  struct addrinfo host_info_2;
  struct addrinfo *host_info_list_2;
  char hostname_mch[NI_MAXHOST];
  struct hostent *aka;
  if (gethostname(hostname_mch, NI_MAXHOST) == -1) {
    perror("gethostname");
    exit(1);
  };
  aka = gethostbyname(hostname_mch);
  if (aka == NULL) {
    perror("gethostbyname");
    exit(1);
  }

  char *hostname_2 = inet_ntoa(*((struct in_addr *)aka->h_addr_list[0]));
  int port2 = 10000;
  memset(&host_info_2, 0, sizeof(host_info_2));

  host_info_2.ai_family = AF_UNSPEC;
  host_info_2.ai_socktype = SOCK_STREAM;
  host_info_2.ai_flags = AI_PASSIVE;

  while (true) {
    status = getaddrinfo(hostname_2, to_string(port2).c_str(), &host_info_2,
                         &host_info_list_2);
    if (status != 0) {
      cerr << "ERROR: cannot addrinfo, error number: " << errno << endl;
      cerr << "  (" << hostname_2 << "," << to_string(port2).c_str() << ")"
           << endl;
      return -1;
    } // if

    socket_fd[1] =
        socket(host_info_list_2->ai_family, host_info_list_2->ai_socktype,
               host_info_list_2->ai_protocol);
    if (socket_fd[1] == -1) {
      cerr << "Error: cannot create socket" << endl;
      cerr << "  (" << hostname_2 << "," << to_string(port2).c_str() << ")"
           << endl;
      return -1;
    } // if
    int yes = 0;
    status =
        setsockopt(socket_fd[1], SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd[1], host_info_list_2->ai_addr,
                  host_info_list_2->ai_addrlen);
    if (status == -1) {
      // cerr << "ERROR: cannot bind, error number: " << endl;
      port2++;
    } else {
      cout << "bind succeed,port is " << to_string(port2).c_str() << endl;
      break;
    }
  }
  const char *port_2 = to_string(port2).c_str();
  status = listen(socket_fd[1], 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname_2 << "," << port_2 << ")" << endl;
    return -1;
  } // if

  cout << "host is " << hostname_2 << endl;
  cout << "Waiting for connection on port " << port_2 << endl;

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

  // get neighbor info from master
  char buffer1[512];
  recv(socket_fd[0], buffer1, 512, 0);
  string neighborInfo(buffer1);
  size_t segment = neighborInfo.find(" ");
  const char *hostname_3 = neighborInfo.substr(0, segment).c_str();
  const char *port_3 = neighborInfo.substr(segment + 1).c_str();

  // const char *msg1 = "Im ready to accept";
  // send(socket_fd[0], msg1, 512, 0);
  // start third socket -- client neighbor

  struct addrinfo host_info_3;
  struct addrinfo *host_info_list_3;
  cout << "Im gonna connect to " << hostname_3 << " " << port_3 << endl;

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
