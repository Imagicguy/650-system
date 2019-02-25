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
  Game(vector<int> socket_fd) { client_fd = socket_fd; }

  void start(int num_hop) {
    hot_potato.is_cold = 0;
    hot_potato.total_hop = num_hop;
    hot_potato.remain_hop = num_hop;
    int recv_bytes;
    bool isSelected = false;
    while (true) {
      FD_ZERO(&fds);
      for (vector<int>::iterator it = client_fd.begin(); it != client_fd.end();
           ++it) {
        FD_SET(*it, &fds);
      }
      int max = *max_element(client_fd.begin(), client_fd.end()) + 1;
      cout << "running" << endl;
      struct timeval tv;
      tv.tv_sec = 10;
      tv.tv_usec = 500000;
      int rv;
      if ((rv = select(max, &fds, NULL, NULL, &tv)) == -1) {
        perror("select");
      } else if (rv == 0) {
        cout << "Timeout occurred! No data after 10.5 seconds" << endl;
      } else {
        cout << "change detected in master!" << endl;
        isSelected = true;
        for (vector<int>::iterator it = client_fd.begin();
             it != client_fd.end(); ++it) {
          if (FD_ISSET(*it, &fds) > 0) {
            recv_bytes =
                recv(*it, &hot_potato, sizeof(hot_potato), MSG_WAITALL);
            if (recv_bytes == -1)
              perror("ERROR: FAILED TO RECV()");

            if (hot_potato.remain_hop == 0) {
              cout << "Trace of potato" << endl;
              for (int i = 0; i < num_hop - 1; i++) {
                printf("%d,", hot_potato.trace[i]);
              }
              printf("%d \n", hot_potato.trace[num_hop - 1]);
              return;
            } else {
              perror("OH,this potato is still hot!");
              return;
            } // master should not get info before hop == 0
          }
        }
      }
      if (isSelected)
        return; // master should only recv() one time
    }
  }

  void pass(int ID) {
    int recv_bytes;
    while (true) {
      FD_ZERO(&fds);
      for (vector<int>::iterator it = client_fd.begin();
           it != client_fd.end() - 1; ++it) {
        FD_SET(*it, &fds);
      }
      struct timeval tv;
      tv.tv_sec = 10;
      tv.tv_usec = 500000;
      int rv;
      int max = *max_element(client_fd.begin(), client_fd.end() - 1) + 1;
      if ((rv = select(max, &fds, NULL, NULL, &tv)) == -1) {
        perror("select");
      } else if (rv == 0) {
        cout << "Timeout occurred! No data after 10.5 seconds" << endl;
      } else {
        for (vector<int>::iterator it = client_fd.begin();
             it != client_fd.end() - 1; ++it) {
          if (FD_ISSET(*it, &fds) > 0) {
            recv_bytes =
                recv(*it, &hot_potato, sizeof(hot_potato), MSG_WAITALL);
            if (recv_bytes == -1)
              perror("ERROR: FAILED TO RECV()");
          }
        }
        if (recv_bytes == 0 || hot_potato.is_cold == 1) {
          for (vector<int>::iterator it = client_fd.begin();
               it != client_fd.end(); ++it) {
            close(*it);
            return;
          }
        }
        hot_potato.trace[hot_potato.total_hop - hot_potato.remain_hop] = ID;
        sleep(2);
        hot_potato.remain_hop--;
        if (hot_potato.remain_hop == 0) { // it is it!
          int send_bytes =
              send(client_fd[0], &hot_potato, sizeof(hot_potato), 0);
          if (send_bytes == -1)
            perror("ERROR : FAILED TO BE IT !");
        } else { // keep passing to random neighbor
          srand((unsigned int)time(NULL));
          int isLeft = rand() % 2;
          if (isLeft) {
            if (send(client_fd[2], &hot_potato, sizeof(hot_potato), 0)) {
              perror("ERROR: FAILED TO SEND TO LEFT NEIGHBOR");
            }
            cout << "Sending to " << ID - 1 << endl;
          } else {
            if (send(client_fd[3], &hot_potato, sizeof(hot_potato), 0)) {
              perror("ERROR: FAILED TO SEND TO LEFT NEIGHBOR");
            }
            cout << "Sending to " << ID + 1 << endl;
          }
        }
      }
    }
  }

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

#endif
