#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
void
usingpipe(){
    int p[2];
    char buf[512];
    pipe(p);

    // //read 终端输入
    // buf[0] = 'a';
    // read(0, buf, sizeof buf);

    // //pipe使用
    // int fd = open("putPipe.txt", O_RDWR);
    // p[0] = dup(fd);
    // int n=read(p[0], buf, sizeof buf);
    
    //pipe 被阻塞
    close(0);
    dup(p[0]);
    read(p[0], buf, sizeof buf);
    //close(p[1]);
    printf("buf = %s", buf);
}

int
main(){
    usingpipe();
    return 0;
}
