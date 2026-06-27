#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * ls | grep .c | wc -l
 */
int main()
{
    int pipe1[2];
    int pipe2[2];

    pipe(pipe1);
    pipe(pipe2);

    if(fork() == 0)
    {
        dup2(pipe1[1], STDOUT_FILENO);

        close(pipe1[0]);
        close(pipe1[1]);

        close(pipe2[0]);
        close(pipe2[1]);

        execlp("ls", "ls", NULL);
        perror("execlp");
        exit(1);
    }

    if(fork() == 0)
    {
        dup2(pipe1[0], STDIN_FILENO);
        dup2(pipe2[1], STDOUT_FILENO);

        close(pipe1[0]);
        close(pipe1[1]);

        close(pipe2[0]);
        close(pipe2[1]);

        execlp("grep", "grep", ".c", NULL);
        perror("execlp");
        exit(1);
    }

    if(fork() == 0)
    {
        dup2(pipe2[0], STDIN_FILENO);

        close(pipe1[0]);
        close(pipe1[1]);

        close(pipe2[0]);
        close(pipe2[1]);

        execlp("wc", "wc", "-l", NULL);
        perror("execlp");
        exit(1);
    }

    close(pipe1[0]);
    close(pipe1[1]);

    close(pipe2[0]);
    close(pipe2[1]);

    wait(NULL);
    wait(NULL);
    wait(NULL);

    return 0;
}