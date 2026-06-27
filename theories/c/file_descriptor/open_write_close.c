#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int fd = open("../../data/data.txt", O_WRONLY | O_TRUNC | O_CREAT, 0640);
    if(fd == -1)
        return 1;
    
    char *txt = "Hello World!\n";
    write(fd, txt, strlen(txt));

    close(fd);

    return 0;
}