// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"
#define BUCKETSIZE 13 // number of hashing buckets
#define BUFFERSIZE 5 // number of available buckets per bucket

extern uint ticks;

// 包含缓冲区缓存的结构体
// struct {
//   // 自旋锁，用于保护缓冲区缓存的数据结构
//   struct spinlock lock;
//   // 缓冲区数组，每个元素都是一个缓冲区
//   struct buf buf[NBUF];

//   // 所有缓冲区的链表，通过prev/next指针连接
//   // 链表按最近使用的顺序排序
//   // head.next是最近使用的缓冲区，head.prev是最久未使用的缓冲区
//   struct buf head;
// } bcache;

struct {
struct spinlock lock;
struct buf buf[BUFFERSIZE];
} bcachebucket[BUCKETSIZE];

int 
hash(uint blockno)
{
  return blockno % BUCKETSIZE;
}


// void
// binit(void)
// {
//   struct buf *b;
//   // 初始化缓存的自旋锁
//   initlock(&bcache.lock, "bcache");

//   // 创建缓冲区的链表
//   bcache.head.prev = &bcache.head;
//   bcache.head.next = &bcache.head;
//   for(b = bcache.buf; b < bcache.buf+NBUF; b++){
//     b->next = bcache.head.next;
//     b->prev = &bcache.head;
//     initsleeplock(&b->lock, "buffer"); // 初始化每个缓冲区的睡眠锁
//     bcache.head.next->prev = b;
//     bcache.head.next = b;
//   }
// }

// 为每个桶初始化一个自旋锁，每个缓冲区一个睡眠锁
void binit(void)
{
  for (int i = 0 ; i < BUCKETSIZE ; i++){
    initlock(&bcachebucket[i].lock,"bcachebucket");
    for (int j = 0 ; j < BUFFERSIZE ; j++){
      initsleeplock(&bcachebucket[i].buf[j].lock,"buffer");
    }
  }
}


// 用于查找设备上的块缓存。
// 如果没有找到，则分配一个新的缓冲块。
// 无论哪种情况，返回被锁定的缓冲块。
// dev代表设备号，用于标识具体的物理设备或磁盘
// blockno表示磁盘上的块号，用于标识设备上的特定块。
// static struct buf*
// bget(uint dev, uint blockno)
// {
//   struct buf *b;

//   acquire(&bcache.lock);

//   // 检查块是否已经在缓存中。
//   for(b = bcache.head.next; b != &bcache.head; b = b->next){
//     if(b->dev == dev && b->blockno == blockno){
//       b->refcnt++;
//       release(&bcache.lock);
//       acquiresleep(&b->lock);
//       return b;
//     }
//   }

//   // 块不在缓存中。
//   // 回收最近最少使用(LRU)的未使用缓冲块。
//   for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
//     if(b->refcnt == 0) {
         // 找到未使用的缓冲块，重新初始化其字段。
//       b->dev = dev;
//       b->blockno = blockno;
//       b->valid = 0;  // 数据无效，需要重新从磁盘读取。
//       b->refcnt = 1; // 设置引用计数为1。
//       release(&bcache.lock);
//       acquiresleep(&b->lock);
//       return b;
//     }
//   }

     // 没有可用的缓冲块，触发恐慌。
//   panic("bget: no buffers");
// }

static struct buf*
bget(uint dev,uint blockno)
{
  struct buf *b;

  int bucket = hash(blockno);
  int finalBuc = bucket;
  acquire(&bcachebucket[bucket].lock);

  for(int i = 0 ; i < BUFFERSIZE ; i++)
  {
    if(bcachebucket[bucket].buf[i].dev == dev && bcachebucket[bucket].buf[i].blockno == blockno){
      b = &bcachebucket[bucket].buf[i];
      b->refcnt++;
      b->timestamp = ticks;
      release(&bcachebucket[bucket].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
  release(&bcachebucket[bucket].lock);

  if(bucket > 0)
  {
    acquire(&bcachebucket[bucket-1].lock);
    for (int i = 0; i < BUFFERSIZE; i++)
    {
      if(bcachebucket[bucket-1].buf[i].dev == dev && bcachebucket[bucket-1].buf[i].blockno == blockno)
      {
        b = &bcachebucket[bucket-1].buf[i];
        b->refcnt++;
        b->timestamp = ticks;
        release(&bcachebucket[bucket-1].lock);
        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcachebucket[bucket-1].lock);
  }

  if(bucket < BUCKETSIZE-1)
  {
    acquire(&bcachebucket[bucket+1].lock);
    for (int i = 0; i < BUFFERSIZE; i++)
    {
      if(bcachebucket[bucket+1].buf[i].dev == dev && bcachebucket[bucket+1].buf[i].blockno == blockno)
      {
        b = &bcachebucket[bucket+1].buf[i];
        b->refcnt++;
        b->timestamp = ticks;
        release(&bcachebucket[bucket+1].lock);
        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcachebucket[bucket+1].lock);    
  }

  uint flag = 0xffffffff;
  int idex = -1;
  acquire(&bcachebucket[bucket].lock);
  for (int i = 0; i < BUFFERSIZE; i++)
  {
    if(bcachebucket[bucket].buf[i].refcnt == 0 && bcachebucket[bucket].buf[i].timestamp < flag)
    {
      flag = bcachebucket[bucket].buf[i].timestamp;
      idex = i;
    }
  }
  release(&bcachebucket[bucket].lock);

  

  // 在邻居 bucket 中获取空闲块
  if(idex == -1)
  {
    if(bucket > 0)
    {
      acquire(&bcachebucket[bucket-1].lock);
      for (int i = 0; i < BUFFERSIZE; i++)
      {
        if(bcachebucket[bucket-1].buf[i].refcnt == 0 && bcachebucket[bucket-1].buf[i].timestamp < flag)
        {
          flag = bcachebucket[bucket-1].buf[i].timestamp;
          idex = i;
          finalBuc = bucket-1;
        }
      }
      release(&bcachebucket[bucket-1].lock);
    }

    if(bucket < BUCKETSIZE-1)
    {
      acquire(&bcachebucket[bucket+1].lock);
      for (int i = 0; i < BUFFERSIZE; i++)
      {
        if(bcachebucket[bucket+1].buf[i].refcnt == 0 && bcachebucket[bucket+1].buf[i].timestamp < flag)
        {
          flag = bcachebucket[bucket+1].buf[i].timestamp;
          idex = i;
          finalBuc = bucket+1;
        }
      }
      release(&bcachebucket[bucket+1].lock);    
    }
  }

  if(idex > -1)
  {
    acquire(&bcachebucket[finalBuc].lock);
    b = &bcachebucket[finalBuc].buf[idex];
    b->dev = dev;
    b->blockno = blockno;
    b->valid = 0;
    b->refcnt = 1;
    release(&bcachebucket[finalBuc].lock);
    acquiresleep(&b->lock);
    return b;
  }

    panic("bget: no buffers");
  
}



// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// 释放锁定的缓冲块。
// 将缓冲块移动到最近最少使用（MRU）链表的头部。
// void
// brelse(struct buf *b)
// {
     // 检查当前线程是否持有缓冲块的睡眠锁。
//   if(!holdingsleep(&b->lock))
//     panic("brelse");

//   releasesleep(&b->lock);

     // 获取全局缓冲区缓存锁，保护共享数据结构。
//   acquire(&bcache.lock);
     // 减少缓冲块的引用计数。
//   b->refcnt--;

     // 如果引用计数为0，表示没有线程在使用这个缓冲块
//   if (b->refcnt == 0) {
//     // 将缓冲块从当前链表位置移除。
//     b->next->prev = b->prev;
//     b->prev->next = b->next;

       // 将缓冲块插入到链表的头部。
//     b->next = bcache.head.next;
//     b->prev = &bcache.head;
//     bcache.head.next->prev = b;
//     bcache.head.next = b;
//   }
  
//   release(&bcache.lock);
// }

void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");
  
  releasesleep(&b->lock);

  int bucket = hash(b->blockno);
  acquire(&bcachebucket[bucket].lock);
  b->refcnt--;
  if(b->refcnt == 0)
    b->timestamp = ticks;
  release(&bcachebucket[bucket].lock);
}


// 增加或减少缓冲块的引用计数
//(用于跟踪缓冲块被引用次数的字段。引用计数的主要目的是确保在缓冲块被使用时不会被其他操作替换或释放，同时也管理缓存块的生命周期。)
// void
// bpin(struct buf *b) {
//   acquire(&bcache.lock);
//   b->refcnt++;
//   release(&bcache.lock);
// }
// void
// bunpin(struct buf *b) {
//   acquire(&bcache.lock);
//   b->refcnt--;
//   release(&bcache.lock);
// }

void
bpin(struct buf *b){
  int bucket = hash(b->blockno);
  acquire(&bcachebucket[bucket].lock);
  b->refcnt++;
  release(&bcachebucket[bucket].lock);
}
void
bunpin(struct buf *b){
  int bucket = hash(b->blockno);
  acquire(&bcachebucket[bucket].lock);
  b->refcnt--;
  release(&bcachebucket[bucket].lock);
}