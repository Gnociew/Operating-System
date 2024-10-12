#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "pstat.h"
#include "stat.h"

//用户程序对 fork 系统调用的请求转发给操作系统内核
int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
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
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
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
    if(myproc()->killed){
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

int sys_settickets(void)
{
  int number;
  // 系统调用包装函数，用于从用户程序传递的参数中获取整数值，并将其存储在指定的变量中
  argint(0,&number);
  if(number < 0)
    return -1;
  else if (number == 0)
    return settickets(InitialTickets);
  else
    return settickets(number);
}

int sys_getpinfo(void)
{
  struct pstat *ps;
  // 从用户空间获取指针起始地址并存入 *ps 中
  if(argptr(0,(char**)&ps,sizeof(struct pstat))<0)
    return -1;
  getpinfo(ps);
  return 0;
}