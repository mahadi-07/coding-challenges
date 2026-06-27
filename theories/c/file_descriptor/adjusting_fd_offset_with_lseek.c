#include <fcntl.h>
#include <stdio.h>
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
    printf("\e[36mfd %d : [\e[0m%s\e[36m]\e[0m\n", fd, buf);
    return;
}

int main()
{
    int fd;
    fd = open("../../data/cat.txt", O_RDONLY);
    if(fd == -1)
        return 1;

    read_and_print_100(fd);
    lseek(fd, 0, SEEK_SET);
    read_and_print_100(fd);

    lseek(fd, 4, SEEK_SET);
    read_and_print_100(fd);

    lseek(fd, 1, SEEK_CUR);
    read_and_print_100(fd);

    lseek(fd, 0, SEEK_END);
    read_and_print_100(fd);

    close(fd);
    return 0;
}
// gcc adjusting_fd_offset_with_lseek.c -o a && ./a && rm a