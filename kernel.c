#include <stdio.h>
#include <string.h>
int main(int argc, char* argv[]) {
    
    // number of processes
    int processesNo = 3;

    int* pids = (int*) malloc(n*sizeof(int));

    int pid = 1;
    int i = 0
    while(pid && i++ <= processesNo) {
		pid = fork();
		if(pid) pids[i] = pid;	
	}

    if(!pid && i == n) {
        // Disk
        // TODO: open disk
        
    }
    else if(!pid) {
        // Process
        // TODO: open process
        // i here means process number

    }
    else {
        // Kernel

    }


    return 0;
}