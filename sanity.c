#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "perf.h"

void
sanity(void)
{
          
  	  int waitingtime = 0, runningtime = 0, turnaroundtime = 0,numOfChilds = 30;
  	  int i = 0,pid[30],start = 0,k = 0,consume=0;
	  int avgWaitingTime = 0, avgRunningTime = 0, avgTurnAroundTime = 0;
	  struct perf performance;
	  
	  for (i = 0; i < 30 ;i = i + 3) {
		    pid[i] = fork();   
		    if(pid[i] < 0) {
				    printf(1,"failed to fork");      
		    }
			      
		    if(pid[i] == 0) {
				    start = uptime();
				    while ((uptime() - start) < 30) {
					consume = consume + 1; //using any command to comsume CPU time.
				    }
				    exit(0);
		    }  

		    pid[i+1] = fork();
		   
		    if(pid[i+1] < 0) {
				    printf(1,"failed to fork");
		    }
			      
		    if(pid[i+1] == 0) {	  	
				    for(k=0; k < 30; k++) {
					  sleep(1);
				    }
				    exit(0);
		    }  
		    
		    pid[i+2] = fork();    
		    if(pid[i+2] < 0) {
				  printf(1,"failed to fork");
		    }
			      
		    if(pid[i+2] == 0) {
				    for(k=0; k < 5; k++) {
					  start = uptime();
					  while((uptime() - start) < 5) {
						  consume = consume + 1; //using any command to comsume CPU time.
					  }				 
					  sleep(1);
				    }
				    exit(0);
		    }	  	    
	  }
	  
  	  for (i = 0; i < 30; i++) {
	         if(pid[i] > 0) {        
			wait_stat(0,&performance);   // Parent process waits here for child to terminate. 
			waitingtime = performance.retime;
			runningtime = performance.rutime;
			turnaroundtime = performance.ttime - performance.ctime;

			avgWaitingTime =    avgWaitingTime + waitingtime;
			avgRunningTime =    avgRunningTime + runningtime;
			avgTurnAroundTime = avgTurnAroundTime + turnaroundtime;

			printf(1,"Child with pid %d has ended. The results are:\n",pid[i]); 
			printf(1,"Waiting time: %d.\n",waitingtime); 
		        printf(1,"Running time: %d.\n",runningtime);           
			printf(1,"Turnaround time: %d.\n\n",turnaroundtime); 	

			if(i == 29) {
			  avgWaitingTime = avgWaitingTime / numOfChilds;
			  avgRunningTime = avgRunningTime / numOfChilds;
			  avgTurnAroundTime = avgTurnAroundTime / numOfChilds;
			  printf(1,"\nAverages:\nWaiting Time: %d.\nRunning Time: %d.\nTurnaround Time: %d.\n", avgWaitingTime, avgRunningTime, avgTurnAroundTime); 
			}
		}
	  }
}

int
main(int argc, char *argv[])
{
  if(argc == 1){
    sanity();
  }
  
  exit(0);
}
