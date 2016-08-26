#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

struct perf {
  int ctime; // Creation
  int ttime; // Termination
  int stime; // SLEEPING
  int retime; // READY
  int rutime; // RUNNING
};

void
sanity(void)
{
  	  int i = 0,pid[30],start = 0,k = 0,consume=0;
	  double avgWaitingTime = 0, avgRunningTime = 0, avgTurnAroundTime = 0;
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
		        printf(0,"got here?");
			wait_stat(0,&performance);   // Parent process waits here for child to terminate. 
			avgWaitingTime =    avgWaitingTime + performance.retime;
			avgRunningTime =    avgRunningTime + performance.rutime;
			avgTurnAroundTime = avgTurnAroundTime + (performance.ttime - performance.ctime);
			printf(4,"Child %d has finished, the results:\n waiting time: %d.\n running time: %d.\n turnaround time: %d.\n\n", pid[i], performance.retime, performance.rutime, (performance.ttime - performance.ctime)); 
	          
		   
		}
	  }
	  avgWaitingTime =    avgWaitingTime / 30;
	  avgRunningTime =    avgRunningTime / 30;
	  avgTurnAroundTime = avgTurnAroundTime / 30;
	  printf(3,"\nAverages:\nWaiting Time: %f.\nRunning Time: %f.\nTurnaround Time: %f.\n", avgWaitingTime, avgRunningTime, avgTurnAroundTime); 
}

int
main(int argc, char *argv[])
{
  if(argc == 1){
    sanity();
  }
  
  exit(0);
}
