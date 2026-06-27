#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main()
{
    int fd;
    int fd_copy;
    int nb_read;
    char buf[15];

    printf("My PID is %d\n", getpid());

    fd = open("../../data/alpha.txt", O_RDONLY);
    if(fd == -1)
        return 1;
    
    fd_copy = dup(fd);
    if(!fd_copy)
        return 1;
    
    nb_read = read(fd, buf, 10);
    if(nb_read == -1)
        return 1;
    buf[nb_read] = '\0';
    printf("fd %d contains : %s\n", fd, buf);

    nb_read = read(fd_copy, buf, 10);
    if(nb_read == -1)
        return 0;
    buf[nb_read] = '\0';
    printf("fd %d contains : %s\n", fd_copy, buf);

    dup2(fd, STDIN_FILENO);
    
    while(1)
        ;
}