#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "perf.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int static policyNumber = 1;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);


int sigdefault(int pid, int signalNum){ 
  cprintf("Default Handler: A signal %d was accepted by process %d\n", signalNum, pid);
  
 *proc->tf = proc->oldtf;
 
 // done handling the signal
 proc->isCurrentlyHandlingSignal = 0;
 
 return 0;
}

int sigreturn(void){ 
 *proc->tf = proc->oldtf;
 
 // done handling the signal
 proc->isCurrentlyHandlingSignal = 0;
 
 return 0;
}


void embeddedSigreturnCall() {
       __asm__ (
          "movl $27, %eax\n\t" // sigreturn number
          "int     $64");
}

void defaultSignalHandler() {
  
   __asm__ (
	  "popl %ecx\n\t" // signal num
	  "popl %ebx\n\t" // pid
          "movl $28, %eax\n\t" // defaultSignalHandler number
          "int     $64");
}

void checkPendingSignals(struct trapframe *tf) {
  // check if we came from user mode 
  // AND we are a process 
  // AND not currently handling any other signal 
  // AND some bit in pending is on
  int i, signum = -1, bit;

  if (proc && (tf->cs&3) == DPL_USER && proc->isCurrentlyHandlingSignal == 0 && (proc->pending != 0) )
  {
  //cprintf("checkPendingSignals pid=%d, %d, %d, %d\n",proc->pid, ((tf->cs&3) == DPL_USER), (proc->isCurrentlyHandlingSignal == 0), (proc->pending != 0));     
    proc->isCurrentlyHandlingSignal = 1;
    for (i = 0; i < NUMSIG; i++) {
      //cprintf("pending: %d\n", proc->pending);
     bit = (1 << i);
     if (proc->pending & bit) {
       signum = i;
       proc->pending = proc->pending & ~bit; // turn off that bit
       break;
     }
    }
    if (signum == -1) {
     return; 
    }
    
      // save the current trapframe because we'll change it
      proc->oldtf = *tf;
      
      if (proc->handlers[signum] == 0) {
	// copy the embedded call to sigreturn syscall to the stack, like task 1.3
	int defaultEmbeddedCodeLength = &checkPendingSignals - &defaultSignalHandler;
	tf->esp -= defaultEmbeddedCodeLength;
	memmove((char*)tf->esp, &defaultSignalHandler, defaultEmbeddedCodeLength);
	int defaultEmbeddedCallEntryPointAdressOnStack = tf->esp;
	
	tf->esp -= 8;
	*(int*)tf->esp = signum; // the parameter for the call
	tf->esp -= 4;
	*(int*)tf->esp = proc->pid; // the parameter for the call
	tf->esp -= 4;
	
	//*(int*)tf->esp = (int)defaultEmbeddedCallEntryPointAdressOnStack; // set the value where esp points to, to point on the embedded code address
	  
	tf->eip = defaultEmbeddedCallEntryPointAdressOnStack;
    } else {
    
    
    // copy the embedded call to sigreturn syscall to the stack, like task 1.3
    int embeddedCodeLength = &defaultSignalHandler - &embeddedSigreturnCall;
    tf->esp -= embeddedCodeLength;
    memmove((char*)tf->esp, &embeddedSigreturnCall, embeddedCodeLength);
    int embeddedCallEntryPointAdressOnStack = tf->esp;
    
    tf->esp -= 4;
    *(int*)tf->esp = signum; // the parameter for the call
    tf->esp -= 4;
    *(int*)tf->esp = (int)embeddedCallEntryPointAdressOnStack; // set the value where esp points to, to point on the embedded code address
    //cprintf("pid: %d, signum: %d. (uint)proc->handlers[signum]: %d, prev eip: %x\n", proc->pid, signum, (uint)proc->handlers[signum], tf->eip);
    
    
    tf->eip = (uint)proc->handlers[signum]; // when switching to user mode (iret at the end of trapasm.s), immediately starts running the handler
    }
     
    
    
  }
}




void initSigHandlers(struct proc * p) {
  int i;
   for (i = 0; i<NUMSIG; i++)
  {
    p->handlers[i] = 0;
  }
}

static unsigned long int next = 1;

int rand(int ticketsSum) // RAND_MAX assumed to be 32767
{ 
    next = next * 1103515245 + 12345;
    int rand = (unsigned int)(next/(2 * (ticketsSum +1)) % (ticketsSum+1));
    return rand ;
}

void srand(unsigned int seed)
{
    next = seed;
}

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

void setProcTicketsNumByPolicy(struct proc *process) {
    int pr;
  switch (policyNumber) {
        case 1:
              process->ntickets = 10;
              break;
        case 2:
              pr = process->priority;
              process->ntickets = pr;
              break;
        case 3:
              process->ntickets = 20;
              break;              
        default:
            break;
   }
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  
  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  p->priority = 10;
  setProcTicketsNumByPolicy(p);
   
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;
  p->ctime = ticks;
  initSigHandlers(p);
  p->isCurrentlyHandlingSignal = 0;
  p->pending = 0;
  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S
 
  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");
      
  p->state = RUNNABLE;
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}



// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);

  safestrcpy(np->name, proc->name, sizeof(proc->name));
 
  pid = np->pid;

  // lock to force the compiler to emit the np->state write last.
  acquire(&ptable.lock);
  np->state = RUNNABLE;
  release(&ptable.lock);
  
  return pid;
}

int
schedp(int policyNum)
{
    struct proc *p;
    policyNumber = policyNum;
    cprintf("The policy is now %d.\n",policyNumber);
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
	  setProcTicketsNumByPolicy(p);
    }
    release(&ptable.lock);
    return 0; 
}

void
priority (int pr)
{
    proc->priority = pr;
    proc->ntickets = proc->priority; // CHANGE
}



// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(int status)
{
  struct proc *p;
  int fd;
  
  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(proc->cwd);
  end_op();
  proc->cwd = 0;
  
  initSigHandlers(proc);
  
  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  proc->exit_status = status;
  proc->ttime = ticks;
  
  

  sched();
  panic("zombie exit");
}

int wait_stat(int * status, struct perf *performance)
{
  struct proc *p;
  int havekids, pid;
  struct perf temp;
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
	if (status!= 0){
	  *status = p->exit_status;
	}
	p->ntickets = 0;
	temp.ctime = p->ctime;
	temp.ttime = p->ttime;
	temp.stime = p->stime;
	temp.retime = p->retime;
	temp.rutime = p->rutime;

        p->stime = 0;
        p->retime = 0;
	p->rutime = 0;

        release(&ptable.lock);

        *performance = temp; 
	 cprintf("Child with pid %d has ended. The results are:\n",pid); 
         return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }  
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(int *status)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
	if (status!= 0){
	  *status = p->exit_status;
	}
	p->ntickets = 0;
	p->pending = 0;
	p->isCurrentlyHandlingSignal = 0;
	initSigHandlers(p);
        release(&ptable.lock);
	
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

int countTickets()
{
  struct proc *p;
 int ticketsSum = 0;
	      
	      for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		  if (p->state == RUNNABLE) {
			  ticketsSum += p->ntickets;
		  }
	      }
	      return ticketsSum;
}


//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
 
  struct proc *p;
  int ticket;
  srand(ticks);
  
     
  for(;;){  
	      // Enable interrupts on this processor.
	      sti();

	      // Loop over process table looking for process to run.
	      acquire(&ptable.lock);

	      int ticketsSum = countTickets();
	      
	      ticket = rand(ticketsSum);
	      
	      for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
		  if(p->state != RUNNABLE)
		      continue;
		  
		  if((ticket - p->ntickets) >= 0) {
			ticket = ticket - p->ntickets;
			continue;
		  }
		  
		  break;
	      }
	      
	      if(p->state == RUNNABLE) {
	      // Switch to chosen process.  It is the process's job
		  // to release ptable.lock and then reacquire it
		  // before jumping back to us.
		  //cprintf("Context switching. pid=%d, pname=%s, p->ntickets=%d, ticket=%d, ticketsSum=%d\n", p->pid, p->name, p->ntickets, ticket, ticketsSum);
		  proc = p;
		  switchuvm(p);
		  p->state = RUNNING;
		  swtch(&cpu->scheduler, proc->context);
		  switchkvm();

		  // Process is done running for now.
		  // It should have changed its p->state before coming back.
		  proc = 0;
	      }
	      release(&ptable.lock);
	     
  }
  
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  if(policyNumber == 3) {           
	if (proc->ntickets > 1) {
	  proc->ntickets -= 1;
	}		    
  }    
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot 
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");


  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

void updatePerformance(void) {
  struct proc *p;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){	      
	    if(p->state == SLEEPING) {
		p->stime++;
		continue;
	    }
	    
	    if (p->state == RUNNABLE) {
		p->retime++;
		continue;
	    } 
	    
	    if (p->state == RUNNING) {
		p->rutime++;
		continue;
	    }
  }
  release(&ptable.lock);
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
	  if(p->state == SLEEPING && p->chan == chan) {
	    p->state = RUNNABLE;
	    if(policyNumber == 3) {           
		  if (p->ntickets < 90) {
			  p->ntickets += 10;
		  }
	    } 
	  }   
  }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}



sighandler_t signal(int signum, sighandler_t handler){
  //cprintf("signal subscriber pid=%d, signum=%d\n", proc->pid, signum);
  acquire(&ptable.lock);
 sighandler_t old = proc->handlers[signum];
 proc->handlers[signum] = handler;
 release(&ptable.lock);
 // if failed, return -1
 return old;
}

int sigsend(int pid, int signum){
  struct proc *p;
  //cprintf("proc.c sigsend pid=%d, %d\n", pid,signum);	
  if (pid > 63 || pid < 0 || signum < 0 || signum > 31) { return -1; } // check if not current process? check process state?
  acquire(&ptable.lock);
   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
     if (p->pid == pid) {
	p->pending = p->pending | (1 << signum); // turn on the signal bit in the receipient process
	//cprintf("sigsend PENDING: %d to %d\n", p->pending, p->pid);	
     }
   }
  release(&ptable.lock);
  
  return 0; // check if no other cases for -1
}


