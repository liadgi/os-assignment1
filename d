[1mdiff --git a/proc.c b/proc.c[m
[1mindex f9ced0b..4b923a2 100644[m
[1m--- a/proc.c[m
[1m+++ b/proc.c[m
[36m@@ -23,7 +23,7 @@[m [mextern void trapret(void);[m
 [m
 [m
 int sigdefault(int pid, int signalNum){ [m
[31m-  cprintf("Default Handler: A signal %d was accepted by process with pid %d.\n", signalNum,pid);[m
[32m+[m[32m  cprintf("Default Handler: A signal %d was accepted by process %d\n", signalNum, pid);[m
   [m
  *proc->tf = proc->oldtf;[m
  [m
[36m@@ -58,78 +58,92 @@[m [mvoid defaultSignalHandler() {[m
           "int     $64");[m
 }[m
 [m
[31m-void initSigHandlers(struct proc * p) {[m
[31m-    int i;[m
[32m+[m[32mvoid checkPendingSignals(struct trapframe *tf) {[m
[32m+[m[32m  // check if we came from user mode[m[41m [m
[32m+[m[32m  // AND we are a process[m[41m [m
[32m+[m[32m  // AND not currently handling any other signal[m[41m [m
[32m+[m[32m  // AND some bit in pending is on[m
[32m+[m[32m  int i, signum = -1, bit;[m
[32m+[m
[32m+[m[32m  if (proc && (tf->cs&3) == DPL_USER && proc->isCurrentlyHandlingSignal == 0 && (proc->pending != 0) )[m
[32m+[m[32m  {[m
[32m+[m[32m  //cprintf("checkPendingSignals pid=%d, %d, %d, %d\n",proc->pid, ((tf->cs&3) == DPL_USER), (proc->isCurrentlyHandlingSignal == 0), (proc->pending != 0));[m[41m     [m
[32m+[m[32m    proc->isCurrentlyHandlingSignal = 1;[m
     for (i = 0; i < NUMSIG; i++) {[m
[31m-            p->handlers[i] = 0;[m
[31m-            //cprintf("p->handlers[i]: %d.\n", p->handlers[i]);[m
[32m+[m[32m      //cprintf("pending: %d\n", proc->pending);[m
[32m+[m[32m     bit = (1 << i);[m
[32m+[m[32m     if (proc->pending & bit) {[m
[32m+[m[32m       signum = i;[m
[32m+[m[32m       proc->pending = proc->pending & ~bit; // turn off that bit[m
[32m+[m[32m       break;[m
[32m+[m[32m     }[m
[32m+[m[32m    }[m
[32m+[m[32m    if (signum == -1) {[m
[32m+[m[32m     return;[m[41m [m
[32m+[m[32m    }[m
[32m+[m[41m    [m
[32m+[m[32m      // save the current trapframe because we'll change it[m
[32m+[m[32m      proc->oldtf = *tf;[m
[32m+[m[41m      [m
[32m+[m[32m      if (proc->handlers[signum] == 0) {[m
[32m+[m	[32m// copy the embedded call to sigreturn syscall to the stack, like task 1.3[m
[32m+[m	[32mint defaultEmbeddedCodeLength = &checkPendingSignals - &defaultSignalHandler;[m
[32m+[m	[32mtf->esp -= defaultEmbeddedCodeLength;[m
[32m+[m	[32mmemmove((char*)tf->esp, &defaultSignalHandler, defaultEmbeddedCodeLength);[m
[32m+[m	[32mint defaultEmbeddedCallEntryPointAdressOnStack = tf->esp;[m
[32m+[m[41m        [m
[32m+[m	[32m //cprintf("proc->pid: %d\n", proc->pid);[m
[32m+[m[41m         [m
[32m+[m	[32mtf->esp -= 4;[m
[32m+[m	[32m*(int*)tf->esp = signum; // the parameter for the call[m
[32m+[m	[32mtf->esp -= 4;[m
[32m+[m	[32m*(int*)tf->esp = proc->pid; // the parameter for the call[m
[32m+[m	[32mtf->esp -= 4;[m
[32m+[m
[32m+[m[32m        //cprintf("*(int*)tf->esp+0 = %x.\n", *((int*)tf->esp+0) );[m
[32m+[m[32m        //cprintf("*(int*)tf->esp+4 = %x.\n", *((int*)tf->esp+1) );[m
[32m+[m[32m        //cprintf("*(int*)tf->esp+8 = %x.\n", *((int*)tf->esp+2) );[m
[32m+[m[32m     //  cprintf("*(int*)tf->esp+12 = %x.\n", *((int*)tf->esp+3) );[m
[32m+[m[32m      // cprintf("*(int*)tf->esp+16 = %x.\n", *((int*)tf->esp+4) );[m
[32m+[m[32m       //cprintf("*(int*)tf->esp+20 = %x.\n", *((int*)tf->esp+5) );[m
[32m+[m	[32m//*(int*)tf->esp = (int)defaultEmbeddedCallEntryPointAdressOnStack; // set the value where esp points to, to point on the embedded code address[m
[32m+[m[41m	  [m
[32m+[m	[32mtf->eip = defaultEmbeddedCallEntryPointAdressOnStack;[m
[32m+[m[32m    } else {[m
[32m+[m[41m    [m
[32m+[m[41m    [m
[32m+[m[32m    // copy the embedded call to sigreturn syscall to the stack, like task 1.3[m
[32m+[m[32m    int embeddedCodeLength = &defaultSignalHandler - &embeddedSigreturnCall;[m
[32m+[m[32m    tf->esp -= embeddedCodeLength;[m
[32m+[m[32m    memmove((char*)tf->esp, &embeddedSigreturnCall, embeddedCodeLength);[m
[32m+[m[32m    int embeddedCallEntryPointAdressOnStack = tf->esp;[m
[32m+[m[41m    [m
[32m+[m[32m    tf->esp -= 8;[m
[32m+[m[32m    *(int*)tf->esp = signum; // the parameter for the call[m
[32m+[m[32m    tf->esp -= 4;[m
[32m+[m[32m    *(int*)tf->esp = (int)embeddedCallEntryPointAdressOnStack; // set the value where esp points to, to point on the embedded code address[m
[32m+[m[32m    //cprintf("pid: %d, signum: %d. (uint)proc->handlers[signum]: %d, prev eip: %x\n", proc->pid, signum, (uint)proc->handlers[signum], tf->eip);[m
[32m+[m[41m    [m
[32m+[m[41m    [m
[32m+[m[32m    tf->eip = (uint)proc->handlers[signum]; // when switching to user mode (iret at the end of trapasm.s), immediately starts running the handler[m
     }[m
[32m+[m[41m     [m
[32m+[m[41m    [m
[32m+[m[41m    [m
[32m+[m[32m  }[m
 }[m
 [m
[31m-void checkPendingSignals(struct trapframe *tf) {[m
[31m-        // check if we came from user mode [m
[31m-        // AND we are a process [m
[31m-        // AND not currently handling any other signal [m
[31m-        // AND some bit in pending is on[m
[31m-        int i, signum = -1, bit;[m
[31m-[m
[31m-        if (proc && (tf->cs&3) == DPL_USER && proc->isCurrentlyHandlingSignal == 0 && (proc->pending != 0) ) {[m
[31m-                        //cprintf("checkPendingSignals pid=%d, %d, %d, %d\n",proc->pid, ((tf->cs&3) == DPL_USER), (proc->isCurrentlyHandlingSignal == 0), (proc->pending != 0));     [m
[31m-                        proc->isCurrentlyHandlingSignal = 1;[m
[31m-                        for (i = 0; i < NUMSIG; i++) {[m
[31m-                                //cprintf("pending: %d\n", proc->pending);[m
[31m-                                bit = (1 << i);[m
[31m-                                if (proc->pending & bit) {[m
[31m-                                        signum = i;[m
[31m-                                        proc->pending = proc->pending & ~bit; // turn off that bit[m
[31m-                                        break;[m
[31m-                                }[m
[31m-                        }[m
[31m-                        if (signum == -1) {[m
[31m-                        return; [m
[31m-                        }[m
[31m-                    [m
[31m-                        // save the current trapframe because we'll change it[m
[31m-                        proc->oldtf = *tf;[m
[31m-                        //cprintf("proc->handlers[signum]: %d.\n",proc->handlers[signum]);  [m
[31m-                       [m
[31m-                        if (proc->handlers[signum] == 0) {[m
[31m-                                        //cprintf("got here ok?");  [m
[31m-                                        // copy the embedded call to sigreturn syscall to the stack, like task 1.3[m
[31m-                                        int defaultEmbeddedCodeLength = &checkPendingSignals - &defaultSignalHandler;[m
[31m-                                        tf->esp -= defaultEmbeddedCodeLength;[m
[31m-                                        memmove((char*)tf->esp, &defaultSignalHandler, defaultEmbeddedCodeLength);[m
[31m-                                        int defaultEmbeddedCallEntryPointAdressOnStack = tf->esp;[m
[31m-                                        [m
[31m-                                        tf->esp -= 8;[m
[31m-                                        *(int*)tf->esp = signum; // the parameter for the call[m
[31m-                                        tf->esp -= 4;[m
[31m-                                        *(int*)tf->esp = proc->pid; // the parameter for the call[m
[31m-                                        tf->esp -= 4;[m
[31m-                                        [m
[31m-                                        //*(int*)tf->esp = (int)defaultEmbeddedCallEntryPointAdressOnStack; // set the value where esp points to, to point on the embedded code address[m
[31m-                                        [m
[31m-                                        tf->eip = defaultEmbeddedCallEntryPointAdressOnStack;[m
[31m-                        } else {  [m
[31m-                                        // copy the embedded call to sigreturn syscall to the stack, like task 1.3[m
[31m-                                        int embeddedCodeLength = &defaultSignalHandler - &embeddedSigreturnCall;[m
[31m-                                        tf->esp -= embeddedCodeLength;[m
[31m-                                        memmove((char*)tf->esp, &embeddedSigreturnCall, embeddedCodeLength);[m
[31m-                                        int embeddedCallEntryPointAdressOnStack = tf->esp;[m
[31m-                                        [m
[31m-                                        tf->esp -= 4;[m
[31m-                                        *(int*)tf->esp = signum; // the parameter for the call[m
[31m-                                        tf->esp -= 4;[m
[31m-                                        *(int*)tf->esp = (int)embeddedCallEntryPointAdressOnStack; // set the value where esp points to, to point on the embedded code address[m
[31m-                                        //cprintf("pid: %d, signum: %d. (uint)proc->handlers[signum]: %d, prev eip: %x\n", proc->pid, signum, (uint)proc->handlers[signum], tf->eip);[m
[31m-                                        [m
[31m-                                        [m
[31m-                                        tf->eip = (uint)proc->handlers[signum]; // when switching to user mode (iret at the end of trapasm.s), immediately starts running the handler[m
[31m-                        } [m
[32m+[m
[32m+[m
[32m+[m
[32m+[m[32mvoid initSigHandlers(struct proc * p) {[m
[32m+[m[32m  int i;[m
[32m+[m[32m   for (i = 0; i<NUMSIG; i++)[m
[32m+[m[32m  {[m
[32m+[m[32m    p->handlers[i] = 0;[m
   }[m
 }[m
 [m
[31m-[m
 static unsigned long int next = 1;[m
 [m
 int rand(int ticketsSum) // RAND_MAX assumed to be 32767[m
[36m@@ -366,7 +380,10 @@[m [mexit(int status)[m
   end_op();[m
   proc->cwd = 0;[m
   [m
[31m-  [m
[32m+[m[32m  initSigHandlers(proc);[m
[32m+[m[32m  proc->pending = 0;[m
[32m+[m[32m  proc->isCurrentlyHandlingSignal = 0;[m
[32m+[m[41m	[m
   acquire(&ptable.lock);[m
 [m
   // Parent might be sleeping in wait().[m
[36m@@ -385,7 +402,7 @@[m [mexit(int status)[m
   proc->state = ZOMBIE;[m
   proc->exit_status = status;[m
   proc->ttime = ticks;[m
[31m-  initSigHandlers(proc);[m
[32m+[m[41m  [m
   [m
 [m
   sched();[m
[36m@@ -421,6 +438,9 @@[m [mint wait_stat(int * status, struct perf *performance)[m
 	  *status = p->exit_status;[m
 	}[m
 	p->ntickets = 0;[m
[32m+[m[32m        p->pending = 0;[m
[32m+[m	[32mp->isCurrentlyHandlingSignal = 0;[m
[32m+[m	[32minitSigHandlers(p);[m
 	temp.ctime = p->ctime;[m
 	temp.ttime = p->ttime;[m
 	temp.stime = p->stime;[m
[36m@@ -431,13 +451,10 @@[m [mint wait_stat(int * status, struct perf *performance)[m
         p->retime = 0;[m
 	p->rutime = 0;[m
 [m
[31m-        initSigHandlers(p);[m
[31m-        p->isCurrentlyHandlingSignal = 0;[m
[31m-        p->pending = 0;[m
         release(&ptable.lock);[m
 [m
         *performance = temp; [m
[31m-	 //cprintf("Child with pid %d has ended. The results are:\n",pid); [m
[32m+[m	[32m// cprintf("Child with pid %d has ended. The results are:\n",pid);[m[41m [m
          return pid;[m
       }[m
     }[m
[36m@@ -484,11 +501,9 @@[m [mwait(int *status)[m
 	  *status = p->exit_status;[m
 	}[m
 	p->ntickets = 0;[m
[31m-        [m
[31m-        initSigHandlers(p);[m
[31m-        p->isCurrentlyHandlingSignal = 0;[m
[31m-        p->pending = 0;[m
[31m-        [m
[32m+[m	[32mp->pending = 0;[m
[32m+[m	[32mp->isCurrentlyHandlingSignal = 0;[m
[32m+[m	[32minitSigHandlers(p);[m
         release(&ptable.lock);[m
 	[m
         return pid;[m
[36m@@ -508,15 +523,14 @@[m [mwait(int *status)[m
 [m
 int countTickets()[m
 {[m
[31m-              struct proc *p;[m
[31m-              int ticketsSum = 0;[m
[32m+[m[32m  struct proc *p;[m
[32m+[m[32m int ticketsSum = 0;[m
 	      [m
 	      for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {[m
 		  if (p->state == RUNNABLE) {[m
 			  ticketsSum += p->ntickets;[m
 		  }[m
 	      }[m
[31m-	      [m
 	      return ticketsSum;[m
 }[m
 [m
[1mdiff --git a/syscall.c b/syscall.c[m
[1mindex 4f9860d..aa23306 100644[m
[1m--- a/syscall.c[m
[1m+++ b/syscall.c[m
[36m@@ -48,6 +48,13 @@[m [margint(int n, int *ip)[m
   return fetchint(proc->tf->esp + 4 + 4*n, ip);[m
 }[m
 [m
[32m+[m[32mint[m
[32m+[m[32margint2(int n, int *ip)[m
[32m+[m[32m{[m
[32m+[m[32m  return fetchint(proc->tf->esp + 1 + 1*n, ip);[m
[32m+[m[32m}[m
[32m+[m
[32m+[m
 // Fetch the nth word-sized system call argument as a pointer[m
 // to a block of memory of size n bytes.  Check that the pointer[m
 // lies within the process address space.[m
[1mdiff --git a/sysproc.c b/sysproc.c[m
[1mindex 898d663..1c1594e 100644[m
[1m--- a/sysproc.c[m
[1m+++ b/sysproc.c[m
[36m@@ -8,10 +8,20 @@[m
 #include "proc.h"[m
 #include "perf.h"[m
 [m
[31m-[m
 int sys_sigdefault(void) {[m
     int pid, signum;[m
[31m-  [m
[32m+[m[32m    //int test, i;[m
[32m+[m[41m    [m
[32m+[m[32m    /*for (i = 0; i< 5; i++) {[m
[32m+[m[32m        if(argint(i, &test) < 0)[m
[32m+[m[32m        {[m
[32m+[m[32m            cprintf("sys_sigdefault test %d failed\n", i);[m
[32m+[m[32m            return -1;[m
[32m+[m[32m        } else {[m
[32m+[m[32m            cprintf("sys_sigdefault test %d arg is %d\n", i, test);[m
[32m+[m[32m        }[m
[32m+[m[41m    [m
[32m+[m[32m    }*/[m
   [m
   if(argint(0, &signum) < 0)[m
   {[m
[36m@@ -19,11 +29,12 @@[m [mint sys_sigdefault(void) {[m
               return -1;[m
   }[m
   [m
[31m-  if(argint(1, &pid) < 0)[m
[32m+[m[32m  if(argint(-1, &pid) < 0)[m[41m [m
   {[m
     cprintf("sys_sigdefault FAILED!!!!!!!\n");[m
               return -1;[m
   }[m
[32m+[m[41m  [m
   sigdefault(pid, signum);[m
  [m
  return 0; [m
