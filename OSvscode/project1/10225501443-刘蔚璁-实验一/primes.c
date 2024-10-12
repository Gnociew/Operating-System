#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


//重定向
//因为父子进程是独立进行的，所以需要重定向实现父子进程之间的通信和数据传递
void mapping(int n, int fd[])
{
    //标准输出：文件描述符为1；标准输入：文件描述符为0
    close(n);
    //dup函数会将指定的文件描述符复制到系统中当前可用的最小文件描述符上
    dup(fd[n]);
    close(fd[0]);
    close(fd[1]);
}

void primes()
{
    int previousP,nextP;
    //如果可以从管道里读到数据
    if(read(0,&previousP,sizeof(int)))
    {
        //管道里的第一个数字一定是素数，直接打印出来
        printf("prime %d\n",previousP);

        //创建新的管道
        int fd[2];
        pipe(fd);

        //创建子进程
        if(fork() == 0)
        {
            //把管道的写入端重定向到标准输出（输出到终端->写入管道）
            mapping(1,fd);
            //如果可以从管道中读到数据
            while (read(0,&nextP,sizeof(int)))
            {
                //且读到的数据不能被第一个读出的数整除
                if(nextP % previousP != 0)
                {
                    //把这个数字写入新创建的管道里
                    //注意！虽然读数据写数据的文件标识符是0和1，但是两个不同的管道
                    write(1,&nextP,sizeof(int));
                }
            }
            
        }
        else
        {
            //子进程结束后执行
            wait(0);
            //把管道的读取端重定向到标准输入（从终端读取->从管道读取）
            mapping(0,fd);
            //递归
            primes();
        }
    }
}

int main(int argc,int *argv[])
{
    //创建管道
    int fd[2];
    pipe(fd);

    //创建子进程
    if(fork() == 0)
    {
        //把管道的写入端重定向到标准输出（输出到终端->写入管道）
        mapping(1,fd);
        for(int i = 2; i <= 35; i++)
        {
            //把数字2-35写入管道中
            write(1,&i,sizeof(int));
        }
    }
    else
    {
        //子进程结束后再执行父进程
        wait(0);
        //把管道的读取端重定向到标准输入（从终端读取->从管道读取）
        mapping(0,fd);
        //调用递归函数
        primes();
    }
    exit(0);
}