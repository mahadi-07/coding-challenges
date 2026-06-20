#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
    pid_t pid = fork();

    if(pid < 0) {
        fprintf(stderr, "Fork failed!\n");
        return 1;
    }
    else if(pid == 0) { /* Code executed ONLY by the child process */
        printf("Hello from the Child Process!\n");
        printf("Child: My PID is %d, Parent PID is %d\n\n", getpid(), getppid());
    }
    else { /* Code executed ONLY by the parent process */
        printf("Hello from the Parent Process!\n");
        printf("Parent: My PID is %d, My Child's PID is %d\n\n", getpid(), pid);
    }

    return 0;
}