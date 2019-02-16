#include <cstring>
#include <iostream>
#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]) {
  // argv[1] = machine_name;
  // argv[2] = port_num;
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = argv[1];
  const char *port = argv[2];

  if (argc < 2) {
    cout << "Syntax: client <hostname>\n" << endl;
    return 1;
  }
}
