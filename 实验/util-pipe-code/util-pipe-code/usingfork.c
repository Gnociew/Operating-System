#include <stdio.h>
#include <stdlib.h>//for exit()
#include <sys/wait.h>//for wait()
#include <unistd.h>//for fork
void
usingfork()
{
    int pid = fork();
    if (pid > 0)
    {
        printf("parent: child=%d\n", pid);
        pid = wait((int *)0);
        printf("child %d is done\n", pid);
    }
    else if (pid == 0)
    {
        printf("child: exiting\n");
        exit(0);
    }
    else
    {
        printf("fork error\n");
    }
}

int
main()
{
    usingfork();
    return 0;
}
