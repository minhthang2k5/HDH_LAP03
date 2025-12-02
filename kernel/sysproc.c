#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

#ifdef LAB_PGTBL
uint64 sys_pgaccess(void)
{
  uint64 baseAdd;
  int len;
  uint64 maskAdd;

  argaddr(0, &baseAdd);
  argint(1, &len);
  argaddr(2, &maskAdd);

  if (len < 0 || len > 64)
    return -1;

  uint64 mask = 0;
  struct proc *p = myproc();
  for (int i = 0; i < len; i++)
  {
    uint64 virMem = baseAdd + i * PGSIZE;
    pte_t *pte = walk(p->pagetable, virMem, 0);

    if (pte != 0 && (*pte & PTE_V) && (*pte & PTE_A))
    {
      mask |= (1L << i);
      *pte &= ~PTE_A;
    }
  }

  if (copyout(p->pagetable, maskAdd, (char *)&mask, sizeof(mask)) < 0)
    return -1;

  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
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
  return xticks;
}
