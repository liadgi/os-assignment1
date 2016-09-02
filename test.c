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

//Test1: Sending all kinds of signals, expected 31 default signal printings!
void test1() {
  printf(1, "Test 1: Sending all kinds of signals, expected 31 default signal printings!\n");
      int father, pid;
      
      father = getpid();
      if ((pid = fork()) == 0) {	
                        int j;
                        for(j = 0; j <= 31; j++) {
                            sigsend(father,j);
                        }
                        exit(0);    
      } else {
	      wait(0);	
      }
      
}


//Test2: Father is registered to all signals with id%3 == 0 (notDefaultHandler will take care of them). His child sends him all signals with id % 3 == 0.
void test2() {
  printf(1, "Test 2: Father is registered to all signals with id%3 == 0 (notDefaultHandler will take care of them). His child sends him all signals with id % 3 == 0.\n");
      int i,fatherID;    
      fatherID = getpid(); 
      
      for (i = 0; i <= 31; i++) {
		      if(i % 3 == 0) {
			  signal(i,notDefaultHandler);
		      }
      }
      
      if(fork() == 0) {
	    for (i = 0; i <= 31; i++) {
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


//Test3: Subscribing all signals to non-default handler, Sending all kinds of signals, expected 31 NON-default signal printings!
void test3() {
  printf(1, "Test 3: Subscribing all signals to non-default handler, Sending all kinds of signals, expected 31 NON-default signal printings!\n");
      int i;
      for(i = 0; i <= 31; i++) {
	  signal(i,notDefaultHandler);
      }
      
      for(i = 0; i <= 31; i++) {
	  sigsend(getpid(),i);
      }
}




int main(void) {
      printf(1, "\n");
      test1();
      printf(1, "\n");
      test2();
      printf(1, "\n");
      test3();

      exit(0);
}