#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
/*
	send signal SIGINT to add message to disk
	send signal SIGUSR1 to get status from disk
	senf signal SIGUSR2 to increment clock of disk
*/



struct msggbuf {
long mtype; // type of message...
char mtext[64]; // data to write.....
int id_to_delete; //id or the slot to free if delete operation.....
int number_of_free_slots; //number of free slots ro send to kernel......
};

int up_stream , down_stream;

void handler(int signum);
void handler1(int signum);
void handler2(int signum);

int pid;
main()
{
	int diskMsgUpQueueId =  msgget(IPC_PRIVATE, 0644);
	if(diskMsgUpQueueId == -1) { printf("failed to create Up message queue...\n"); return 0;}
	up_stream = diskMsgUpQueueId;
	int diskMsgDownQueueId =  msgget(IPC_PRIVATE, 0644); 
	if(diskMsgDownQueueId == -1) {printf("failed to create Down message queue...\n"); return 0;}
	down_stream = 	diskMsgDownQueueId;	 
	signal (SIGINT, handler);
	signal (SIGUSR1, handler1);
	signal (SIGUSR2, handler2);
	pid = fork(); 
	if(pid==0){
	
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
	else{
	    while(1) {
		
 	    }       
	}	

 
}


//receiving status from disk.....
void handler1(int signum){
  msggbuf msg;
  printf("OUCH....\nSignal #%d received\n",signum);
  kill(pid , SIGUSR1);
  sleep(1);
  int received = msgrcv(up_stream,&msg,sizeof msg,0,IPC_NOWAIT);
  if(received != -1)printf("number of slots = %d\n" , msg.number_of_free_slots);	
  
}

// incrementing clock of disk....
void handler2(int signum){
  kill(pid , SIGUSR2); 
}

//sending message add or delete to disk...
void handler(int signum)
{
  char arr[64] = "abcdefghi";
  msggbuf msg;
  msg.mtype = 1;
  strcpy(msg.mtext,arr) ;
  printf("OUCH....\nSignal #%d received\n",signum);
  int sent = msgsnd(down_stream , &msg , sizeof msg ,  IPC_NOWAIT );
  if(sent != -1) printf("sent successfully...!\n");
}
