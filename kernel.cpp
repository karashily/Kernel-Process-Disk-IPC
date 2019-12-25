#include<bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
using namespace std;
int clk=0;
int number_of_killed_processes;

struct msggbuf {
long mtype; // type of message...
char mtext[64]; // data to write.....
int id_to_delete; //id or the slot to free if delete operation.....
int number_of_free_slots; //number of free slots ro send to kernel......
};
void handler(int signum);


int main() {

    signal (SIGCHLD, handler);


    // number of processes
    int processesNo =2;
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
//    printf("%d\n" , pid);
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

	//check if there all processed terminated and there are no messages to receive from processes or send to disk then terminate...
	struct msqid_ds buf1 , buf2;
  	int num_messages_to_disk , num_messages_from_processes;
  	int rc1 = msgctl(diskMsgDownQueueId , IPC_STAT, &buf1);
	int rc2 = msgctl(processMsgUpQueueId , IPC_STAT, &buf2);
  	num_messages_to_disk = buf1.msg_qnum;
	num_messages_from_processes = buf2.msg_qnum;
	if(number_of_killed_processes == processesNo && num_messages_to_disk == 0 &&num_messages_from_processes == 0 ) break;
	
	printf("\n----at clock %d---- \n" , clk);
        //receiving message from processes....
        msggbuf received_msg;
        int receive = msgrcv(processMsgUpQueueId,&received_msg,sizeof received_msg,0,IPC_NOWAIT);
	      if(receive == -1) printf("Kernel: no messages from processes...!\n");
        else{
          //identify type pf message....
          //add
          if(received_msg.mtype == 1){
              //check status of disk.....
              kill(pids[processesNo+1],SIGUSR1);
	      //get status
              msggbuf status;
              int chk = msgrcv(diskMsgUpQueueId,&status,sizeof status,0,!IPC_NOWAIT);
              if(chk == -1){
                printf("Kernel: failed to get disk status....!\n" );
              }
              else{
                //valid...
                if(status.number_of_free_slots > 0){
                  //add
                  msggbuf msg;
                  msg.mtype = 1;
                  strcpy(msg.mtext , received_msg.mtext);
                  int add = msgsnd(diskMsgDownQueueId , &msg , sizeof msg ,  !IPC_NOWAIT );

                }
                else{
                  printf("Kernel: there is no space in memory.....!\n");
                }
              }
          }
          else{
              //delete....
              msggbuf msg;
              msg.mtype = 2;
              msg.id_to_delete = received_msg.id_to_delete;
              int del = msgsnd(diskMsgDownQueueId , &msg , sizeof msg ,  !IPC_NOWAIT );
              if(del == -1){
                printf("Kernel: failed to delete....!\n");
              }
              else{
                printf("Kernel: delete request is successful....\n" );
            }
        }
      }
      sleep(1);
      // Kernel
      // use pids array to send clk signal to all processes
      for(int j = 1 ; j<=processesNo+1 ; j++){
        kill(pids[j] , SIGUSR2);
      }
      clk++;
    
     }
   kill(pids[processesNo+1] , 9);
   printf("Disk terminated.....!\n");
  }

	  printf("Kernel terminated....!\n");
    return 0;
}

void handler(int signum)
{

 //printf("Children have sent a SIGCHLD signal #%d\n",signum);

 while (1) {
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid <= 0) {
        break;
    
    }
    number_of_killed_processes++;
    //if(!(status & 0x00FF))
    //printf("\nA child with pid %d terminated with exit code %d\n", pid, stat_loc>>8);
  }

}

