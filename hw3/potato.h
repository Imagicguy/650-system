
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
  Game(vector<int> socket_fd) { client_fd = socket_fd; }

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
      struct timeval tv;
      tv.tv_sec = 10;
      tv.tv_usec = 500000;
      int rv;
      if ((rv = select(max, &fds, NULL, NULL, NULL)) == -1) {
        perror("select");
      } else {

        isSelected = true;
        for (vector<int>::iterator it = client_fd.begin();
             it != client_fd.end(); ++it) {
          if (FD_ISSET(*it, &fds) > 0) {
            recv_bytes = recv(*it, &hot_potato, sizeof(potato), 0);
            if (recv_bytes < 0)
              perror("ERROR: FAILED TO RECV()");

            if (hot_potato.remain_hop == 0) {
              cout << "Trace of potato:" << endl;
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

  void pass(int ID, int num_players) {
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

      if ((rv = select(max, &fds, NULL, NULL, NULL)) == -1) {
        perror("select");
      } else {

        for (vector<int>::iterator it = client_fd.begin();
             it != client_fd.end() - 1; ++it) {

          if (FD_ISSET(*it, &fds) > 0) {

            // cout << "before recv" << hot_potato.remain_hop << endl;
            recv_bytes = recv(*it, &hot_potato, sizeof(potato), 0);
            // cout << "wtf is " << (char)hot_potato << endl;
            // char buffe[50000];
            // recv_bytes = recv(*it, buffe, 50000, 0);
            // cout << "wtf is " << buffe << endl;
            //     cout << "after recv" << hot_potato.remain_hop << endl;

            if (recv_bytes == -1)
              perror("ERROR: FAILED TO RECV()");
          }
        }

        if (hot_potato.is_cold == 1) {
          for (vector<int>::iterator it = client_fd.begin();
               it != client_fd.end(); ++it) {
            close(*it);
            return;
          }
        }
        hot_potato.trace[hot_potato.total_hop - hot_potato.remain_hop] = ID;
        sleep(1);
        hot_potato.remain_hop--;
        if (hot_potato.remain_hop == 0) { // it is it!
          int send_bytes = send(client_fd[0], &hot_potato, sizeof(potato), 0);
          if (send_bytes < 0)
            perror("ERROR : FAILED TO BE IT !");
          cout << "I'm it" << endl;
        } else { // keep passing to random neighbor
          srand((unsigned int)time(NULL) + ID);
          int isLeft = rand() % 2;
          if (isLeft == 1) {
            // cout <<  "nanshou " client_fd[2] << endl;
            int bytes = send(client_fd[1], &hot_potato, sizeof(potato), 0);
            if (bytes < 0) {
              perror("ERROR: FAILED TO SEND TO LEFT NEIGHBOR");
            }
            int offset = ID == 0 ? num_players : ID;
            cout << "Sending potato to " << offset - 1 << endl;
          } else {
            int bytes = send(client_fd[2], &hot_potato, sizeof(potato), 0);
            if (bytes < 0) {
              perror("ERROR: FAILED TO SEND TO LEFT NEIGHBOR");
            }
            int offset = ID == num_players - 1 ? -1 : ID;
            cout << "Sending potato to " << offset + 1 << endl;
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
