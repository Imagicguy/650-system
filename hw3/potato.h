#ifndef __POTATO_H__
#define __POTATO_H__
#include <algorithm>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
using namespace std;

typedef struct potato_t {
  int total_hop;
  int remain_hop;
  vector<int> trace;
  int is_cold;
} potato_t;

typedef struct potato_t potato;

class Game {
private:
  fd_set fds;
  potato hot_potato;
  vector<int> client_fd;

public:
  Game(int *client, int len) {
    for (int i = 0; i < len; i++) {
      client_fd.push_back(client[i]);
    }
  }

  void start(int num_hop) {
    hot_potato.is_cold = 0;
    hot_potato.total_hop = num_hop;
    hot_potato.remain_hop = num_hop;
    int recv_bytes;
    while (true) {
      FD_ZERO(&fds);
      for (vector<int>::iterator it = client_fd.begin(); it != client_fd.end();
           ++it) {
        FD_SET(*it, &fds);
      }
      int max = *max_element(client_fd.begin(), client_fd.end()) + 1;
      if (select(max, &fds, NULL, NULL, NULL) > 0) {
        for (vector<int>::iterator it = client_fd.begin();
             it != client_fd.end(); ++it) {
          if (FD_ISSET(*it, &fds) > 0) {
            recv_bytes = recv(*it, &hot_potato, sizeof(hot_potato), 0);
            if (recv_bytes == -1)
              perror("ERROR: FAILED TO RECV()");

            if (hot_potato.remain_hop == 0) {
              cout << "Trace of potato" << endl;
              for (int i = 0; i < num_hop - 1; i++) {
                printf("%d,", hot_potato.trace[i]);
              }
              printf("%d \n", hot_potato.trace[num_hop - 1]);
              break;
            } else {
              perror("OH,this potato is still hot!");
              break;
            } // master should not get info before hop == 0
          }
        }
      }
      break; // master should only recv() one time
    }
  }

  void passing(int num_hop) {}
  void over() {
    hot_potato.is_cold = 1;
    for (vector<int>::iterator it = client_fd.begin(); it != client_fd.end();
         ++it) {
      int succeed = send(*it, &hot_potato, sizeof(hot_potato), 0);
      if (succeed == -1)
        perror("Failed to stop this game!");
    }
  }
};
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
