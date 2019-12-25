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

const int N = (int) 1e5;
int clk;
pair<string , string> msgs[N];
int existed[N];

struct msggbuf {
long mtype; // type of message...
char mtext[64]; // data to write.....
int id_to_delete; //id or the slot to free if delete operation.....
int number_of_free_slots; //number of free slots ro send to kernel......
};

void handler2(int signum);

int main(int argc, char* argv[]) {

   signal (SIGUSR2, handler2);
   int mx_clk = 0;
   int processNo = atoi(argv[1]);
   int msgUpQueueId = atoi(argv[2]);
   int msgDownQueueId = atoi(argv[3]);
   
   //char processNoStr[12];
   //sprintf(processNoStr, "%d", processNo);

   string fileName = "file";
   fileName.append(to_string(processNo));
   fileName.append(".txt");
   ifstream cin;
   cin.open(fileName);

   //freopen(fileName , "r" , stdin);
   string s;
   while(getline(cin , s)){
     stringstream ss(s);
     string w,ww; ss>>w;
     int number_clk = stoi(w);
     mx_clk = max(mx_clk , number_clk);
     existed[number_clk] = 1;
     ss >> w;
     ss >> ww;
     msgs[number_clk].first = w;
     msgs[number_clk].second = ww;
 
   }

   while(clk <= mx_clk){
     // there is a message to be sent at this time.........
     if(existed[clk]){
	existed[clk] = 0;
       //send message via up stream...
       msggbuf msg;
       if(msgs[clk].first == "ADD"){
         msg.mtype = 1;
	 //cout<< &msgs[clk].second[0]<<endl;
         strcpy(msg.mtext , &msgs[clk].second[0]);
       }
       else{
         msg.mtype = 2;
         msg.id_to_delete = stoi(msgs[clk].second);
       }
       int sent = msgsnd(msgUpQueueId , &msg , sizeof msg , !IPC_NOWAIT);
       if(sent == -1) printf("process #%d failed to send message...\n" , processNo);
     }
   }
}


void handler2(int signum){
  clk++;
  //printf("My Pid is [%d], and i have got signal #%d to increment my clock %d.......\n",getpid(), signum ,clk);
}

