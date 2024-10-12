// Saved registers for kernel context switches.
struct context {
  uint64 ra;
  uint64 sp;

  // callee-saved
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

// Per-CPU state.
struct cpu {
  struct proc *proc;          // The process running on this cpu, or null.
  struct context context;     // swtch() here to enter scheduler().
  int noff;                   // Depth of push_off() nesting.
  int intena;                 // Were interrupts enabled before push_off()?
};

extern struct cpu cpus[NCPU];

// per-process data for the trap handling code in trampoline.S.
// sits in a page by itself just under the trampoline page in the
// user page table. not specially mapped in the kernel page table.
// the sscratch register points here.
// uservec in trampoline.S saves user registers in the trapframe,
// then initializes registers from the trapframe's
// kernel_sp, kernel_hartid, kernel_satp, and jumps to kernel_trap.
// usertrapret() and userret in trampoline.S set up
// the trapframe's kernel_*, restore user registers from the
// trapframe, switch to the user page table, and enter user space.
// the trapframe includes callee-saved user registers like s0-s11 because the
// return-to-user path via usertrapret() doesn't return through
// the entire kernel call stack.
struct trapframe {
  /*   0 */ uint64 kernel_satp;   // kernel page table
  /*   8 */ uint64 kernel_sp;     // top of process's kernel stack
  /*  16 */ uint64 kernel_trap;   // usertrap()
  /*  24 */ uint64 epc;           // saved user program counter
  /*  32 */ uint64 kernel_hartid; // saved kernel tp
  /*  40 */ uint64 ra;
  /*  48 */ uint64 sp;
  /*  56 */ uint64 gp;
  /*  64 */ uint64 tp;
  /*  72 */ uint64 t0;
  /*  80 */ uint64 t1;
  /*  88 */ uint64 t2;
  /*  96 */ uint64 s0;
  /* 104 */ uint64 s1;
  /* 112 */ uint64 a0;
  /* 120 */ uint64 a1;
  /* 128 */ uint64 a2;
  /* 136 */ uint64 a3;
  /* 144 */ uint64 a4;
  /* 152 */ uint64 a5;
  /* 160 */ uint64 a6;
  /* 168 */ uint64 a7;
  /* 176 */ uint64 s2;
  /* 184 */ uint64 s3;
  /* 192 */ uint64 s4;
  /* 200 */ uint64 s5;
  /* 208 */ uint64 s6;
  /* 216 */ uint64 s7;
  /* 224 */ uint64 s8;
  /* 232 */ uint64 s9;
  /* 240 */ uint64 s10;
  /* 248 */ uint64 s11;
  /* 256 */ uint64 t3;
  /* 264 */ uint64 t4;
  /* 272 */ uint64 t5;
  /* 280 */ uint64 t6;
};

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
// 维护一个进程的状态，其中最为重要的状态是进程的页表，内核栈，当前运行状态
struct proc {
  struct spinlock lock;       // 进程锁，用于保护进程结构的并发访问

  // p->lock must be held when using these:
  enum procstate state;        // 进程当前的运行状态
  void *chan;                  // 进程等待的通道，如果非空，则表示进程正在休眠等待某个事件发生
  int killed;                  // 表示进程是否被杀死
  int xstate;                  // 返回给父进程 wait 的退出状态
  int pid;                     // 进程 ID，唯一标识一个进程


  // wait_lock must be held when using this:
  struct proc *parent;         // 父进程指针，指向当前进程的父进程

  // these are private to the process, so p->lock need not be held.
  uint64 kstack;               // 内核栈的虚拟地址，用于保存进程的内核执行栈
  uint64 sz;                   // 进程内存的大小（字节），表示进程占用的内存空间大小
  pagetable_t pagetable;       // 用户页表，用于管理进程的虚拟地址空间
  struct trapframe *trapframe; // 用户态中断帧，用于保存用户态执行的上下文信息
  struct context context;      // 进程上下文，用于进行进程间的上下文切换
  struct file *ofile[NOFILE];  // 打开文件数组，用于存储进程打开的文件描述符
  struct inode *cwd;           // 当前工作目录
  struct usyscall *usyscall;   // 指向用户态系统调用结构的指针
  char name[16];               // 进程名字（用于调试）
};