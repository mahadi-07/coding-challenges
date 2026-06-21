#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 100

void read_and_print_100(int fd)
{
    char buf[BUFFER_SIZE + 1];
    int nb_read;

    nb_read = read(fd, buf, BUFFER_SIZE);
    if(nb_read == -1) {
        printf("Read error!\n");
        return;
    }

    buf[nb_read] = '\0';
    printf("\e[36mfd %d : [\e[0m%s\e[36m]\e[0m\n\n", fd, buf);
    return;
}

int main()
{
    printf("%d\n\n", getpid());

    int fd1;
    int fd2;

    fd1 = open("../../data/cat.txt", O_RDONLY);
    fd2 = open("../../data/cat.txt", O_RDONLY);

    if(fd1 == -1 || fd2 == -1)
        return 1;

    read_and_print_100(fd1);
    read_and_print_100(fd1);
    read_and_print_100(fd2);

    getchar();

    close(fd1);
    close(fd2);



    return 0;
}

// gcc resetting_fd_offset.c -o a && ./a && rm a