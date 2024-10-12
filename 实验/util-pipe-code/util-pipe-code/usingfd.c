#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

void usingfd()
{
    char buf[512];
    int fd = dup(0);
    close(0);
    int fd2=open("input.txt", O_RDONLY);
    int n = read(0, buf, sizeof buf);
    write(1, buf, sizeof buf);
}
int 
main(){
    usingfd();
    return 0;
}