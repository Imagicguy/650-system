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
  int trace[512];
  int is_cold;
} potato_t;

typedef struct potato_t potato;

class Game {
private:
  fd_set fds;
  // potato hot_potato;
  vector<int> client_fd;

public:
  Game(int *client, int len, int num_hop) {
    for (int i = 0; i < len; i++) {
      client_fd.push_back(client[i]);
    }
    /*for (int i = 0; i < 512; i++) {
      hot_potato.trace[i] = -1;
    }
    hot_potato.is_cold = 0;
    hot_potato.total_hop = num_hop;
    hot_potato.remain_hop = num_hop;*/
  }
  Game(vector<int> socket_fd) {
    client_fd = socket_fd;
    /*for (int i = 0; i < 512; i++) {
      hot_potato.trace[i] = -1;
    }
    hot_potato.is_cold = 0;
    hot_potato.total_hop = -1;
    hot_potato.remain_hop = -1;*/
  }

  void start(int num_hop) {
    potato hot_potato;
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
        for (int i = 0; i < max; i++) {
          if (FD_ISSET(i, &fds) > 0) {
            recv_bytes = recv(i, &hot_potato, sizeof(potato), MSG_WAITALL);
            if (recv_bytes == -1)
              perror("ERROR: FAILED TO RECV()");

            if (hot_potato.remain_hop == 0) {
              cout << "Trace of potato" << endl;
              for (int j = 0; j < num_hop - 1; j++) {
                printf("%d,", hot_potato.trace[j]);
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
    potato hot_potato;
    hot_potato.total_hop = 22;
    hot_potato.remain_hop = 21;
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
      cout << "2" << endl;
      if ((rv = select(max, &fds, NULL, NULL, &tv)) == -1) {
        perror("select");
      } else if (rv == 0) {
        cout << "Timeout occurred! No data after 10.5 seconds" << endl;
      } else {
        int count = 0;
        for (vector<int>::iterator it = client_fd.begin();
             it != client_fd.end() - 1; ++it) {

          if (FD_ISSET(*it, &fds) > 0) {
            recv_bytes = recv(*it, &hot_potato, sizeof(potato), MSG_WAITALL);
            cout << "this is " << count << endl;
            cout << "in select" << hot_potato.total_hop << endl;
            cout << "in select" << hot_potato.remain_hop << endl;
            if (recv_bytes == -1)
              perror("ERROR: FAILED TO RECV()");
          }
          count++;
        }

        if (recv_bytes == 0 || hot_potato.is_cold == 1) {
          for (vector<int>::iterator it = client_fd.begin();
               it != client_fd.end(); ++it) {
            close(*it);
            return;
          }
        }
        cout << "3" << endl;
        cout << hot_potato.total_hop << endl;
        cout << hot_potato.remain_hop << endl;
        hot_potato.trace[hot_potato.total_hop - hot_potato.remain_hop] = ID;
        cout << "4" << endl;
        sleep(2);
        hot_potato.remain_hop--;
        if (hot_potato.remain_hop == 0) { // it is it!
          int send_bytes = send(client_fd[0], &hot_potato, sizeof(potato), 0);
          if (send_bytes == -1)
            perror("ERROR : FAILED TO BE IT !");
        } else { // keep passing to random neighbor
          srand((unsigned int)time(NULL));
          int isLeft = rand() % 2;
          if (isLeft) {
            if (send(client_fd[2], &hot_potato, sizeof(potato), 0)) {
              perror("ERROR: FAILED TO SEND TO LEFT NEIGHBOR");
            }
            cout << "Sending to " << ID - 1 << endl;
          } else {
            if (send(client_fd[3], &hot_potato, sizeof(potato), 0)) {
              perror("ERROR: FAILED TO SEND TO LEFT NEIGHBOR");
            }
            cout << "Sending to " << ID + 1 << endl;
          }
        }
      }
    }
  }

  void over() {
    potato hot_potato;
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
