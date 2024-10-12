#include"types.h"
#include"stat.h"
#include"user.h"

int main(int argc,char *argv[])
{
    //将字符串转换为int
    int number = atoi(argv[1]);
    settickets(number);

    // 保持进程的运行状态
    while (1)
    {
    }

    exit();
}