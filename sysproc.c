#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "perf.h"

int sys_sigdefault(void) {
    int pid, signum;
    //int test, i;
    
    /*for (i = 0; i< 5; i++) {
        if(argint(i, &test) < 0)
        {
            cprintf("sys_sigdefault test %d failed\n", i);
            return -1;
        } else {
            cprintf("sys_sigdefault test %d arg is %d\n", i, test);
        }
    
    }*/
  
  if(argint(0, &signum) < 0)
  {
    cprintf("sys_sigdefault FAILED!!!!!!!\n");
              return -1;
  }
  
  if(argint(-1, &pid) < 0) 
  {
    cprintf("sys_sigdefault FAILED!!!!!!!\n");
              return -1;
  }
  
  sigdefault(pid, signum);
 
 return 0; 
}

int sys_sigreturn(){ 
  return sigreturn();
 return 0; 
}

int sys_sigsend(void){
  //change
  
  int pid, signum;
  if(argint(0, &pid) < 0)
  {
    cprintf("SYS_sigsend FAILED!!!!!!!\n");
              return -1;
  }
  
  if(argint(1, &signum) < 0)
  {
    cprintf("SYS_sigsend FAILED!!!!!!!\n");
              return -1;
  }
  
  //cprintf("SYS_sigsend pid=%d, %d\n", pid,signum);	
  return sigsend(pid, signum);
}

sighandler_t sys_signal(void) {
  int signum;
  char* handler;
  //sighandler_t handler;
  int pointerSize = 4;
  
  if(argint(0, &signum) < 0)
              return (sighandler_t)-1;
  
  if(argptr(1, &handler , pointerSize) < 0)
              return (sighandler_t)-1;
  
 return signal(signum, (sighandler_t)handler);
}

int
sys_fork(void)
{
  return fork();
}




int 
sys_wait_stat(void)
{
    int n = 0;
    int status;
    char* performance;
  
     if(argint(0, &status) < 0)
              return -1;
     
     if(argptr(1, &performance,n) < 0)
              return -1;
     
     return wait_stat(&status, (struct perf*)performance);
}

int 
sys_schedp(void)
{
    int policyNum;
    argint(0, &policyNum);  
 
    return schedp(policyNum);
}

void
sys_priority(void)
{
  int pr;
 if(argint(0, &pr) < 0)
    exit( -1);
 priority(pr);
  
}


int
sys_exit(void)
{
  int status;
  if (argint(0, &status) < 0) { // there is another thing on the stack
    exit(-1); //cannot access stack
  }
  
  exit(status);
  return 0;  // not reached
}

int
sys_wait(void)
{
  int *status;
 if(argint(0, (int*)&status) < 0)
    return -1;
 //cprintf("sys_wait: proc=%d-%s, status=%d\n", proc->pid, proc->name,status);
  return wait(status);
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

