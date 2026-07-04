#include <unistd.h>
#include <stdio.h>
#include <signal.h>

void handler(int sig)
{
    printf("Pressed %d\n", sig);
    
    printf("sleep handler method\n");
    sleep(10);
    printf("wake up\n");
}

/**
 * We should use `sigaction` instead of `signal` because it has better defined
 * semantics. `signal` on different operating system does different things which
 * is bad. `sigaction` is more portable and is better defined for threads.
 * We can use system call `sigaction` to set the current handler and disposition
 * for a `signal` or read the current signal handler for a particular signal.
 */
int main()
{
    struct sigaction sa = {0};
    sigemptyset(&sa.sa_mask);

    // Block SIGQUIT while handler executes
        // will execute after handler finished it execution
    sigaddset(&sa.sa_mask, SIGQUIT);

    sa.sa_handler = handler;
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);

    for(int i = 0; i < NSIG; i++)
        if(sigismember(&sa.sa_mask, i))
            printf("Signal %d is in the set\n", i);

    while(1) {
        printf("...\n");
        sleep(1);
    }

    return 0;
}