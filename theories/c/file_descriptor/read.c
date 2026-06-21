#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 100

int main()
{
    char buf[BUFFER_SIZE + 1];
    int fd;
    int nb_read;
    int count;

    fd = open("../../data/cat.txt", O_RDONLY);
    if(fd == -1)
        return 1;
    
    nb_read = -1;
    count = 0;

    while(nb_read != 0) {
        printf("Current position :%ld\n", (long) lseek(fd, 0, SEEK_CUR));

        nb_read = read(fd, buf, BUFFER_SIZE);
        if(nb_read == -1) {
            printf("Read error!\n");
            return 1;
        }
        buf[nb_read] = '\0';

		printf("\e[36m%d : [\e[0m%s\e[36m]\e[0m\n", count, buf);
        count++;
    }

    close(fd);
    return 0;
}