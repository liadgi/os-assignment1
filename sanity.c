#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "perf.h"

void
sanity(void)
{   
  	  int waitingtime = 0, runningtime = 0, turnaroundtime = 0,numOfChilds = 30,donePID,childPID;
  	  int i = 0,pid[30],start = 0,passed = 0,k = 0,consume=0;
	  int avgWaitingTime = 0, avgRunningTime = 0, avgTurnAroundTime = 0;
          int avgWaitingTime1 = 0, avgRunningTime1 = 0, avgTurnAroundTime1 = 0;
          int avgWaitingTime2 = 0, avgRunningTime2 = 0, avgTurnAroundTime2 = 0;
          int avgWaitingTime3 = 0, avgRunningTime3 = 0, avgTurnAroundTime3 = 0;
	  struct perf performance;
	  
	  for (i = 0; i < 30 ;i = i + 1) {
                                    pid[i] = fork(); 
                                    if(pid[i] < 0) {
                                                    printf(1,"failed to fork");      
                                    }

                                    if(pid[i] == 0) { 
                                                childPID = getpid();     
                                                if(childPID % 3 == 0) {         
                                                                //printf(0,"\n\nChild of type 1 is now running.\n\n"); 
                                                                start = uptime();
                                                                passed = uptime();
                                                                while ((passed - start) < 30) {
                                                                    consume = consume + 1; //using any command to comsume CPU time.
                                                                    consume = consume * 1; //using any command to comsume CPU time.
                                                                    consume = consume % 1; //using any command to comsume CPU time.
                                                                    passed = uptime();
                                                                }
                                                                exit(0);
                                                }
                                                
                                                if(childPID % 3 == 1) {        
                                                                //printf(0,"\n\nChild of type 2 is now running.\n\n"); 
                                                                for(k=0; k < 30; k++) {
                                                                        sleep(1);
                                                                }
                                                                exit(0);
                                                }
                                                
                                                if(childPID % 3 == 2) {          
                                                                //printf(0,"\n\nChild of type 3 is now running.\n\n"); 
                                                                for(k=0; k < 5; k++) {
                                                                        start = uptime();
                                                                        passed = uptime();
                                                                        while((passed - start) < 5) {
                                                                                consume = consume + 1; //using any command to comsume CPU time.
                                                                                consume = consume * 1; //using any command to comsume CPU time.
                                                                                consume = consume % 1; //using any command to comsume CPU time.
                                                                                passed = uptime();
                                                                        }				 
                                                                        sleep(1);
                                                                }
                                                                exit(0);
                                                }
                                    }
			  
	  }     
	  
  	  for (i = 0; i < 30; i++) {       
			  donePID =  wait_stat(0,&performance);   // Parent process waits here for child to terminate. 
			  waitingtime = performance.retime;
			  runningtime = performance.rutime;
			  turnaroundtime = performance.ttime - performance.ctime;
                          
			  avgWaitingTime =    avgWaitingTime + waitingtime;
			  avgRunningTime =    avgRunningTime + runningtime;
			  avgTurnAroundTime = avgTurnAroundTime + turnaroundtime;

                          if(donePID%3 == 0) {
                                avgWaitingTime1 =    avgWaitingTime1 + waitingtime;
                                avgRunningTime1 =    avgRunningTime1 + runningtime;
                                avgTurnAroundTime1 = avgTurnAroundTime1 + turnaroundtime;                                 
                          }
                          
                          if(donePID%3 == 1) {
                                avgWaitingTime2 =    avgWaitingTime2 + waitingtime;
                                avgRunningTime2 =    avgRunningTime2 + runningtime;
                                avgTurnAroundTime2 = avgTurnAroundTime2 + turnaroundtime;                                                              
                          }
                          
                          if(donePID%3 == 2) {
                                avgWaitingTime3 =    avgWaitingTime3 + waitingtime;
                                avgRunningTime3 =    avgRunningTime3 + runningtime;
                                avgTurnAroundTime3 = avgTurnAroundTime3 + turnaroundtime;                                                 
                          }
                          
			  printf(1,"Child with pid %d has ended. The results are:\n",donePID); 
			  printf(1,"Waiting time: %d.\n",waitingtime); 
			  printf(1,"Running time: %d.\n",runningtime);           
			  printf(1,"Turnaround time: %d.\n\n",turnaroundtime); 	

			  if(i == 29) {
			    avgWaitingTime = avgWaitingTime / numOfChilds;
			    avgRunningTime = avgRunningTime / numOfChilds;
			    avgTurnAroundTime = avgTurnAroundTime / numOfChilds;
                            
			    avgWaitingTime1 = avgWaitingTime1 / 10;
			    avgRunningTime1 = avgRunningTime1 / 10;
			    avgTurnAroundTime1 = avgTurnAroundTime1 / 10;
                            
 			    avgWaitingTime2 = avgWaitingTime2 / 10;
			    avgRunningTime2 = avgRunningTime2 / 10;
			    avgTurnAroundTime2 = avgTurnAroundTime2 / 10;
                            
                            avgWaitingTime3 = avgWaitingTime3 / 10;
			    avgRunningTime3 = avgRunningTime3 / 10;
			    avgTurnAroundTime3 = avgTurnAroundTime3 / 10;
                            
			    printf(1,"\nAverages:\nWaiting Time: %d.\nRunning Time: %d.\nTurnaround Time: %d.\n", avgWaitingTime, avgRunningTime, avgTurnAroundTime); 
 			    printf(1,"\nChilds Type 1 ONLY:\nAverages:\nWaiting Time: %d.\nRunning Time: %d.\nTurnaround Time: %d.\n", avgWaitingTime1, avgRunningTime1, avgTurnAroundTime1); 
			    printf(1,"\nChilds Type 2 ONLY:\nAverages:\nWaiting Time: %d.\nRunning Time: %d.\nTurnaround Time: %d.\n", avgWaitingTime2, avgRunningTime2, avgTurnAroundTime2); 
			    printf(1,"\nChilds Type 3 ONLY:\nAverages:\nWaiting Time: %d.\nRunning Time: %d.\nTurnaround Time: %d.\n", avgWaitingTime3, avgRunningTime3, avgTurnAroundTime3); 
                            
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
