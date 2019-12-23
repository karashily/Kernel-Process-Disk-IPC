#include<bits/stdc++.h>
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
int clk;
struct msggbuf {
long mtype; // type of message...
char mtext[64]; // data to write.....
int id_to_delete; //id or the slot to free if delete operation.....
int number_of_free_slots; //number of free slots ro send to kernel......
};

int main() {

    // number of processes
    int processesNo =1;
    //atoi(argv[1]);


    // TODO: create two message queues for each of the process and disk
    int processMsgUpQueueId;
    int processMsgDownQueueId;
    int diskMsgUpQueueId;
    int diskMsgDownQueueId;

    diskMsgUpQueueId =  msgget(IPC_PRIVATE, 0644);
  	if(diskMsgUpQueueId == -1) { printf("failed to create Up message queue...\n"); return 0;}
  	diskMsgDownQueueId =  msgget(IPC_PRIVATE, 0644);
  	if(diskMsgDownQueueId == -1) {printf("failed to create Down message queue...\n"); return 0;}
    processMsgUpQueueId =  msgget(IPC_PRIVATE, 0644);
  	if(processMsgUpQueueId == -1) { printf("failed to create Up message queue...\n"); return 0;}
  	processMsgDownQueueId =  msgget(IPC_PRIVATE, 0644);
  	if(processMsgDownQueueId == -1) {printf("failed to create Down message queue...\n"); return 0;}

    int* pids = (int*) malloc(processesNo * sizeof(int));

    int pid = 1;
    int i = 0;
    while(pid && i++ <= processesNo) {
		pid = fork();
		if(pid) pids[i] = pid;
	}

    if(!pid && i == processesNo + 1) {
        // Disk

        char* args[4];

        char* diskFileName = "./disk.out";

        char upQueueId[12];
        sprintf(upQueueId, "%d", diskMsgUpQueueId);

        char downQueueId[12];
        sprintf(downQueueId, "%d", diskMsgDownQueueId);

        args[0] = diskFileName;
        args[1] = upQueueId;
        args[2] = downQueueId;
        args[3] = NULL;

        execvp(args[0], args);
        printf("Starting Disk Process Failed\n");
        return 1;
    }
    else if(!pid) {
        // Process
        // i here means process number

        char* args[5];

        char* processFileName = "./process.out";

        char processNo[12];
        sprintf(processNo, "%d", i);

        char upQueueId[12];
        sprintf(upQueueId, "%d", processMsgUpQueueId);

        char downQueueId[12];
        sprintf(downQueueId, "%d", processMsgDownQueueId);

        args[0] = processFileName;
        args[1] = processNo;
        args[2] = upQueueId;
        args[3] = downQueueId;
        args[4] = NULL;

        execvp(args[0], args);
        printf("Starting process no. %d failed\n", i);
        return 2;
    }
    else {

      while(true){
        //receiving message from processes....
        msggbuf received_msg;
        int receive = msgrcv(processMsgUpQueueId,&received_msg,sizeof received_msg,0,IPC_NOWAIT);
        if(receive == -1) printf("there is no messages from processes...!\n");
        else{
          //identify type pf message....
          //add
          if(received_msg.mtype == 1){
              //check status of disk.....
              kill(pids[processesNo],SIGUSR1);
              sleep(1);
              //get status
              msggbuf status;
              int chk = msgrcv(diskMsgUpQueueId,&status,sizeof status,0,IPC_NOWAIT);
              if(chk == -1){
                printf("failed to get disk status....!\n" );
              }
              else{
                //valid....
                if(status.number_of_free_slots > 0){
                  //add
                  msggbuf msg;
                  msg.mtype = 1;
                  strcpy(msg.mtext , received_msg.mtext);
                  int add = msgsnd(diskMsgDownQueueId , &msg , sizeof msg ,  IPC_NOWAIT );
                }
                else{
                  printf("there is no space in memory.....!\n");
                }
              }
          }
          else{
              //delete....
              msggbuf msg;
              msg.mtype = 2;
              msg.id_to_delete = received_msg.id_to_delete;
              int del = msgsnd(diskMsgDownQueueId , &msg , sizeof msg ,  IPC_NOWAIT );
              if(del == -1){
                printf("failed to delete....!\n");
              }
              else{
                printf("deleted request is successful....\n" );
            }
        }
      }

      // Kernel
      // use pids array to send clk signal to all processes
      for(int i = 1 ; i<=processesNo ; i++){
        kill(pids[i] , SIGUSR2);
      }
      clk++;
      sleep(1);


    }
  }


    return 0;
}

