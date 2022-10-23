#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "processinfo.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  
  if(myproc()->trace_mask >> 2) {
    printf("Args : ");
    printf("%d ", n);
    printf("\n");
  }

  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  if(myproc()->trace_mask >> 11) {
    printf("Args : ");
    printf("\n");
  }
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  if(myproc()->trace_mask >> 1) {
      printf("Args : ");
      printf("\n");
    }
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;

  if(myproc()->trace_mask >> 4) {
    printf("Args : ");
    printf("%p ", (void*)p);
    printf("\n");
  }

  return wait(p);
}

//Added P2
/*
uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;

  if(myproc()->trace_mask >> 12) {
    printf("Args : ");
    printf("%d", n);
    printf("\n");
  }

  return addr;
}
*/

//Added P2
uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0) {
    return -1;
  }
  struct proc *p = myproc();
  addr = p->sz;
  p->sz+=n;
  if (n < 0) {
    uvmdealloc(p->pagetable, addr, p->sz);
  }
  return addr;
}


uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  printf("Sleeping for %d ticks\n", n);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);

  if(myproc()->trace_mask >> 13) {
    printf("Args : ");
    printf("%d ", n);
    printf("\n");
  }

  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  
  if(myproc()->trace_mask >> 6) {
    printf("Args : ");
    printf("%d ", pid);
    printf("\n");
  }

  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);

  if(myproc()->trace_mask >> 14) {
    printf("Args : ");
    printf("\n");
  }

  return xticks;
}

uint64
sys_echo_simple(void)
{
  const int MAX_LENGTH = 1024;
  char str[MAX_LENGTH];
  if(argstr(0,str,MAX_LENGTH) < 0) { 
  	return -1;
  }
  printf("%s\n",str);

  if(myproc()->trace_mask >> 22) {
    printf("Args : ");
    printf("%s ", str);
    printf("\n");
  }

  return 0;
}

uint64
sys_echo_kernel(void)
{
  const int MAX_LENGTH = 1024;
  char addr[MAX_LENGTH];
  uint64 uargv, uarg;
  int i;

  if(argaddr(1, &uargv) < 0){
    return -1;
  }
  for(i = 1; ; i++){
    if(fetchaddr(uargv+sizeof(uint64)*i, (uint64*)&uarg) < 0){
      return -1;
    }
    if(uarg == 0){
      break;
    }
    if(addr == 0)
      return -1;
    if(fetchstr(uarg, addr, MAX_LENGTH) < 0)
      return -1;
    printf("%s ", addr);
  }
  printf("\n");
  
  if(myproc()->trace_mask >> 23) {
    printf("Args : ");
    printf("%p ", (void*)uargv);
    printf("\n");
  }

  return 0;
}

uint64
sys_trace(void)
{
    struct proc* p = myproc(); 
    if(argint(0, &p->trace_mask) < 0) {
      return -1;
    }
    if(myproc()->trace_mask >> 24) {
      printf("Args : ");
      printf("%d ", p->trace_mask);
      printf("\n");
    }
    return 0;
}

uint64
sys_get_process_info(void)
{
  uint64 addr;

  if(argaddr(0, &addr) < 0) {
    return -1;
  }
  
  struct proc *p = myproc();
  struct processinfo temp_p;
  for(int i = 0; i < 16; i++) {
    temp_p.name[i] = p->name[i];
  }
  temp_p.pid = p->pid;
  temp_p.sz = p->sz;
  if(copyout(p->pagetable, addr, (char*)&temp_p, sizeof(temp_p)) < 0) {
    return -1;
  }
  if(myproc()->trace_mask >> 25) {
      printf("Args : ");
      printf("\n");
  }

  return 0;
}