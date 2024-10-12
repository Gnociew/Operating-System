// Per-CPU state
struct cpu {
  uchar apicid;                // 本地 APIC 的 ID，用于多处理器系统中识别每个处理器
  struct context *scheduler;   // 指向 context 结构体的指针，用于在进程调度时执行上下文切换
  struct taskstate ts;         // 由 x86 架构使用的任务状态段，用于在中断时找到中断处理程序所需的堆栈
  struct segdesc gdt[NSEGS];   // x86 全局描述符表（Global Descriptor Table），用于管理内存段的访问权限和属性
  volatile uint started;       //  CPU 是否已经启动的标志
  int ncli;                    //  pushcli 函数的调用深度，用于跟踪中断嵌套
  int intena;                  // 在调用 pushcli 之前中断是否被启用的标志
  struct proc *proc;           // 指向当前 CPU 上运行的进程的指针，如果没有正在运行的进程则为 null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc {
uint sz;                     // 表示进程的内存大小，以字节为单位
pde_t* pgdir;                // 指向页表的指针，用于管理进程的虚拟内存
char *kstack;                // 指向内核栈底部的指针，用于保存进程在内核态执行时的栈信息
enum procstate state;        // 表示进程的状态，是一个枚举类型
int pid;                     // 进程的唯一标识符，即进程ID
struct proc *parent;         // 指向父进程的指针
struct trapframe *tf;        // 指向当前系统调用的陷阱帧的指针，用于保存进程在用户态被中断时的寄存器状态
struct context *context;     // 指向上下文切换信息的指针，用于保存进程在内核态被中断时的上下文信息
void *chan;                  // 如果进程正在等待某个条件（如等待文件I/O完成），则指向该条件的指针
int killed;                  // 如果非零，表示进程已经被杀死
struct file *ofile[NOFILE];  // 表示进程打开的文件数组
struct inode *cwd;           // 指向当前工作目录的指针
char name[16];               // 进程名称，用于调试目的
int ticket;                  //进程彩票数
int tick;                    //时间片数
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
