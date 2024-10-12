#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

//从路径中提取文件名 
//从字符串"a/b/c/d"中获取字符串"d",基本原理为从后往前找到第一个'/'
char* fmtname(char *path) 
{
  //DIRSIZ:文件名的最大长度
  static char buf[DIRSIZ+1];
  char *p;

  //从最后一个字符往前循环，直到遇到'/'
  for(p=path+strlen(path); p >= path && *p != '/'; p--);
  p++;

  //疑问：为什么要分情况返回p或者缓冲区指针？（经测试直接返回p也可以）
  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  //将字符串从指针p指向的位置复制到缓冲区buf中
  memmove(buf, p, strlen(p));
  buf[strlen(p)] = '\0';

  return buf;
}

//比较两个文件名是否相同并打印路径
void cmp(char *a, char *b)  
{
    //strcmp 函数会逐个比较两个字符串的字符，
    //如果在某个位置上两个字符不相同，则返回它们的 ASCII 差值；
    //如果两个字符串完全相同，则返回 0。
    if(!strcmp(fmtname(a), b))
        printf("%s\n", a);
}

void
find(char *path, char *target)
{
  //缓冲区buf用来储存路径
  char buf[512], *p;
  //文件描述符
  int fd;
  //存的是目录底下的文件名或者子目录信息
  struct dirent de;
  //文件的元数据信息（文件的基本属性，包括文件所在文件系统、文件的类型、大小等信息） 
  struct stat st;

  //打开指定路径终点的文件，并返回一个文件描述符。0：只读
  if((fd = open(path, 0)) < 0){ 
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  //获取一个已打开文件的状态信息，并将其存储stat中
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
    //该文件为文件时，若匹配成功，结束
  case T_FILE:
    cmp(path, target);
    break;
    
    //该文件为文件夹时
  case T_DIR: 

    //如果路径过长
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }

    //把当前路径写到缓冲区中
    strcpy(buf, path);
    //指针指向路径末尾
    p = buf+strlen(buf);
    *p++ = '/';
    
    //通过循环从已经打开的目录文件中读取目录条目
    //de存的是目录底下的文件名或者子目录信息
    while(read(fd, &de, sizeof(de)) == sizeof(de)){

      //若文件夹下文件数量为0，1或者该文件夹名字为"."或".."则不进入，防止套娃
      /*de.inum 是目录项的 inode 号。
      在 Unix 系统中，inode 号为 0 表示这个目录项未被使用，inode 号为 1 表示这个目录的父目录。*/
      /*de.name 是目录项的名称。
      .（点）代表当前目录，..（点点）代表父目录。*/
      if(de.inum == 0 || de.inum == 1 || strcmp(de.name, ".")==0 || strcmp(de.name, "..")==0)
        continue;
      
      //将文件名追加到路径，递归find
      memmove(p, de.name, strlen(de.name)); 
      p[strlen(de.name)] = '\0';
      find(buf, target);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{


    if(argc < 3){
        printf("Please input the right arg!\n");
        exit(0);
    }

    find(argv[1], argv[2]);
    exit(0);
}