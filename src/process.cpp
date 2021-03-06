#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
using namespace std;

const int N = (int)1e5;
int clk;
pair<string, string> msgs[N];
int existed[N];

struct msggbuf {
  long mtype;               // type of message...
  char mtext[64];           // data to write.....
  int id_to_delete;         //id or the slot to free if delete operation.....
  int number_of_free_slots; //number of free slots ro send to kernel......
};

void handler2(int signum);

int main(int argc, char *argv[]) {

  signal(SIGUSR2, handler2);
  
  // reading arguments sent from kernel
  int processNo = atoi(argv[1]);
  int msgUpQueueId = atoi(argv[2]);
  int msgDownQueueId = atoi(argv[3]);

  // reading file
  string fileName = "file";
  fileName.append(to_string(processNo));
  fileName.append(".txt");
  ifstream cin;
  cin.open(fileName);

  int mx_clk = 0;
  string s;
  while (getline(cin, s)) {
    stringstream ss(s);
    string w, ww;
    ss >> w;
    int number_clk = stoi(w);
    mx_clk = max(mx_clk, number_clk);
    existed[number_clk] = 1;
    ss >> w;
    ss >> ww;
    msgs[number_clk].first = w;
    msgs[number_clk].second = ww;
  }

  // sending messages; each in its time
  while (clk <= mx_clk) {
    if (existed[clk]) { 
      // there is a message to be sent at this time.........

      existed[clk] = 0;
      
      //send message via up stream...
      msggbuf msg;
      if (msgs[clk].first == "ADD") {
        msg.mtype = 1;
        strcpy(msg.mtext, &msgs[clk].second[0]);
      }
      else {
        msg.mtype = 2;
        msg.id_to_delete = stoi(msgs[clk].second);
      }
      int sent = msgsnd(msgUpQueueId, &msg, sizeof msg, !IPC_NOWAIT);
      if (sent == -1) {
        printf("Process #%d: failed to send message %s...\n", processNo, msg.mtext);
      }
    }
  }
  printf("Process #%d terminated..!\n", processNo);
}

void handler2(int signum) {
  clk++;
}