#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

void *thread_func(void *arg)
{
    sleep(3);
    printf("Hello from thread!\n");
    
    long status = 1;
    return (void *) status; // Pass the 1 directly inside the pointer register

}

int main()
{
    pthread_t tid;

    if(pthread_create(&tid, NULL, thread_func, NULL) != 0)
    {
        perror("pthread_create");
        return 1;
    }

    printf("Before pthread_join()\n");

    void *status;
    pthread_join(tid, &status);

    printf("Thread finished %ld\n", (long) status);

    return 0;
}

// gcc create_thread.c -o a -pthread && ./a && rm a