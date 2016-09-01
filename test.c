#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "perf.h"

void hellio(int signal) {  
      printf(1,"test \n", signal);
}

void notDefaultHandler(int signal) {  
      printf(1,"Running non-default handler (Handled Signal number is: %d)\n", signal);
}

//Test1: Subscribing all signals to non-default handler, Sending all kinds of signals, expected 31 NON-default signal printings!
void test1() {
      int i;
      for(i = 0; i <= 31; i++) {
	  signal(i,notDefaultHandler);
      }
      
      for(i = 0; i <= 31; i++) {
	  sigsend(getpid(),i);
      }
}

//Test2: Sending all kinds of signals, expected 31 default signal printings!
void test2() {
      int father, pid;
      
      father = getpid();
      
      if((pid = fork()) == 0) {
	printf(1, "here");
	      int j;
	      for(j = 0; j <= 31; j++) {
		  sigsend(father,j);
	      }
	      exit(0);
      } else {
	printf(1, "there");
	    sleep(100);
	      wait(0);	
      }
}

//Test3: Father is registered to all signals with id%3 == 0 (notDefaultHandler will take care of them). His child sends him all signals with id % 3 == 0.
void test3() {
      int i,fatherID;    
      fatherID = getpid(); 
      
      for (i = 0; i <= 31; i++) {
		      if(i % 3 == 0) {
			  signal(i,notDefaultHandler);
		      }
      }
      
      if(fork() == 0) {
	    for (i = 0; i < 30; i++) {
	         if(i % 3 == 0) {
		      if(sigsend(fatherID,i) < 0) {
			    printf(1, "Failed sending signal %d to father with pid %d",i,fatherID);
		      }
		 }
	    }
	    exit(0);
      } else {	     
	    wait(0);
      }  
}


int main(void) {
      test1();
      //test2();
      //test3();
      exit(0);
}