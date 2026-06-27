#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void writestr(int fd, const char *str)
{
    write(fd, str, strlen(str));
}

int main()
{
    int pipefd[2];
    pid_t pid;
    char buf;

    if(pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if(pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0) {
        close(pipefd[1]);
        writestr(STDOUT_FILENO, "Child: What is the secret in this pipe?\n");
        writestr(STDOUT_FILENO, "Child: \n");

        while(read(pipefd[0], &buf, 1) > 0) {
            write(STDOUT_FILENO, &buf, 1);
        }

        writestr(STDOUT_FILENO, "\"\n");
        writestr(STDOUT_FILENO, "Child: Wow! I must go see my father.\n");

        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }
    else {
        close(pipefd[0]);
        writestr(STDOUT_FILENO, "Parent: I'm writing a secret in this pipe...\n");
        writestr(pipefd[1], "\e[33mI am your father mwahahaha!\e[0m");
        close(pipefd[1]);

        wait(NULL);

        writestr(STDOUT_FILENO, "Parent: Hellow child!\n");
        exit(EXIT_SUCCESS);
    }
}