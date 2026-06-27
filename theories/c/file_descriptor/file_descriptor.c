#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd = open("../../data/data1.txt", O_CREAT);
    if(fd == -1) {
        perror("Can't open the file");
        return 1;
    }
    unlink("../../data/data1.txt");
    return 0;
}