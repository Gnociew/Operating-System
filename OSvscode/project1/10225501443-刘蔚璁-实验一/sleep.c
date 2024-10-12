#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc,char *argv[])
/*argc和argv[]：用于接收命令行参数的参数列表
    argc：命令行参数的数量
    argv[]：指针数组，用于存储每个命令行参数的字符串
*/
{
    /*如果命令行参数小于2，打印错误信息*/
    if(argc<2){
        fprintf(2, "Usage: sleep number\n");
        exit(1);
    }

    /*把传入的第二个参数通过atoi函数转化为整数*/
    int number = atoi(argv[1]);

    /*调用sleep函数，使程序休眠number秒*/
    sleep(number);
    
    exit(0);
} 