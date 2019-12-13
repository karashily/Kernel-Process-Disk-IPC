#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    
    // number of processes
    int processesNo = atoi(argv[1]);


    // TODO: create two message queues for each of the process and disk
    int processMsgUpQueueId;
    int processMsgDownQueueId;
    int diskMsgUpQueueId;
    int diskMsgDownQueueId;
    
    
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
        // Kernel
        // use pids array to send clk signal to all processes


    }


    return 0;
}