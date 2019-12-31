#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>

int clk;
int free_slots = 10;
int up_stream;
int down_stream;

const int N = 10;
const int M = 64;
// 10 slots whith max 64 chars
char slots[N][M];

struct msggbuf {
  long mtype;               // type of message...
  char mtext[64];           // data to write.....
  int id_to_delete;         //id or the slot to free if delete operation.....
  int number_of_free_slots; //number of free slots ro send to kernel......
};

void handler2(int signum);
void handler1(int signum);

int main(int argc, char *argv[]) {

  signal(SIGUSR1, handler1);
  signal(SIGUSR2, handler2);

  // reading arguments sent from kernel
  int msgUpQueueId = atoi(argv[1]);
  int msgDownQueueId = atoi(argv[2]);
  up_stream = msgUpQueueId;
  down_stream = msgDownQueueId;

  while (true) {
    msggbuf msg;
    //receiving meassage......
    int received = msgrcv(msgDownQueueId, &msg, sizeof msg, 0, !IPC_NOWAIT);
    if (received != -1) {
      if (msg.mtype == 1) {
        //add operation......
        
        int current_clock = clk;
        
        // searching for a free slot........
        bool added = false;
        for (int i = 0; i < N; ++i)
        {
          // check if free....
          if (!strlen(slots[i]))
          {
            printf("Disk: message %s is being added at slot %d.....!\n", msg.mtext, i);
            strcpy(slots[i], msg.mtext);
            free_slots--;
            added = true;
            while (current_clock + 3 > clk); //wait 3 seconds............
            printf("Disk: message %s added successfully at slot %d.....!\n", msg.mtext, i);
            break;
          }
        }
        if (!added) {
          printf("Disk: no free slots for message %s\n", msg.mtext);
        }
      }
      else if (msg.mtype == 2) {
        //delete operation........
      
        int current_clock = clk;
        
        // id of the slot to free..........
        int id = msg.id_to_delete;
        
        printf("Disk: message at slot %d is being deleted..!\n", id);
        
        //check if it's not already deleted.............
        if (strlen(slots[id])) {
          free_slots++; //incrementing number of free slots...............
          for (int i = 0; i < 64; ++i) {
            slots[id][i] = '\0';
          }
        }
        while (current_clock + 1 > clk); //wait 1 second............
        printf("Disk: message at slot %d deleted successfully..!\n", id);
      }
    }
    else {
      printf("Disk: no messages...!\n");
    }
  }
}

void handler2(int signum) {
  clk++;
}

void handler1(int signum) {
  msggbuf msg;
  msg.number_of_free_slots = free_slots;
  msg.mtype = 1; //type of status message on the up stream.......
  int sent = msgsnd(up_stream, &msg, sizeof msg, IPC_NOWAIT);
}
