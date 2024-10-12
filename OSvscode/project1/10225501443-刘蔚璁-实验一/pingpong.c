#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*双管道实现*/
//   int main(int argc, char *argv[]) {
//     // 定义两个整型数组，用于存放管道的文件描述符
//     int p1[2];  //ping
//     int p2[2];  //pong

//     // 创建两个管道
//     pipe(p1);
//     pipe(p2);

//     // 创建子进程
//     int pid = fork();

//     if (pid == 0) {
//         // 关闭不需要的文件描述符
//         close(p1[1]);   //ping的写入端
//         close(p2[0]);   //pong的读取段

//         // 存放子进程接收到的数据
//         char son[3];     
//         read(p1[0], son, 4); 
//         close(p1[0]);

//         // 打印子进程接收到的信息
//         printf("%d: received %s\n",getpid(),son);

//         write(p2[1], "pong", 4); 
//         close(p2[1]);
//     } 
    
//     else if (pid > 0) {
    
//         close(p1[0]);   //ping的读取端
//         close(p2[1]);   //pong的写入端

//         write(p1[1], "ping", 4); 
//         close(p1[1]);

//         // 存放父进程接收到的数据
//         char father[3];   
//         read(p2[0], father, 4); 
//         close(p2[0]);

//         // 打印父进程接收到的信息
//         printf("%d: received %s\n",getpid(),father);
//     }

//     // 终止程序，确保不会有其他未知行为
//     exit(0);
//   }


  int main(int argc,char* argv[]){
    //创建一个管道，实现ping、pong的读写
    //创建成功后，p[0]为管道读取端，p[1]为管道写入端
    int p[2];
    pipe(p);

    char readtext[10];//存储父进程和子进程的读出内容

    //创建子进程
    //在父进程中folk返回子进程的pid，在子进程中folk返回0
    int pid = fork();

    if(pid==0){
        read(p[0],readtext,10);
        printf("%d: received %s\n",getpid(),readtext);
        write(p[1],"pong",10);
        exit(0);//子进程一定要退出
    }
    
    else{
        write(p[1],"ping",10);
        //父进程阻塞，等待子进程读取
        wait(0);
        read(p[0],readtext,10);
        printf("%d: received %s\n",getpid(),readtext);
    }

    exit(0);
}