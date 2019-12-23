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

struct msggbuf {
long mtype; // type of message...
char mtext[64]; // data to write.....
int id_to_delete; //id or the slot to free if delete operation.....
int number_of_free_slots; //number of free slots ro send to kernel......
};

void handler2(int signum);
void handler1(int signum);

int main(int argc, char* argv[]) {

    signal (SIGUSR1, handler1);
    signal (SIGUSR2, handler2);
    int msgUpQueueId = atoi(argv[1]);
    int msgDownQueueId = atoi(argv[2]);
    up_stream = msgUpQueueId;
    down_stream = msgDownQueueId;
    const int N = 10;
    const int M = 64;
    // 10 slots whith max 64 chars
    char slots[N][M];
    msggbuf msg;
    while(true){
      //receiving meassage......
      int received = msgrcv(msgDownQueueId,&msg,sizeof msg,0,!IPC_NOWAIT);
      if(received != -1){
        //add operation......
        if(msg.mtype == 1){
          free_slots--;
          int current_clock = clk;
          // searching for a free slot........
          for(int i = 0 ; i < 10 ; ++i){
            // check if free....
            if(!strlen(slots[i])){
              strcpy(slots[i] , msg.mtext);
              
            }
          }
	   printf("message %s added successfully.....!\n" , msg.mtext);
           while(current_clock+3 > clk){} //wait 3 seconds............
        }
        //delete operation........
        else if(msg.mtype == 2){
          int current_clock = clk;
          // id of the slot to free..........
          int id = msg.id_to_delete;
          //check if already deleted.............
          if(strlen(slots[id])) free_slots++; //incrementing number of free slots...............
          for(int i = 0 ; i < 64 ; ++i){
            slots[id][i] = '\0';
          }
	  printf("message at slot %d deleted successfully..\n" , id);
          while(current_clock+1 > clk){} //wait 1 second............
        }

      }
      else{
        printf("no messages\n");
      }
    }

}

void handler2(int signum){
  clk++;
  printf("My Pid is [%d], and i have got signal #%d to increment my clock %d.......\n",getpid(), signum ,clk);
}

void handler1(int signum){
  printf("here is my status(number of free slots)......\n");
  msggbuf msg;
  msg.number_of_free_slots = free_slots;
  msg.mtype = 1; //type of status message on the up stream.......
  int sent = msgsnd(up_stream , &msg , sizeof msg , !IPC_NOWAIT);
  printf("My Pid is [%d], and i have got signal #%d to send my status to kerenl.....\n",getpid(), signum);
}

