#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void *worker(void *arg)
{
    sleep(2);
    printf("Worker finished\n");
    return NULL;
}

int main(void)
{
    pthread_t tid;

    pthread_create(&tid, NULL, worker, NULL);
    pthread_detach(tid);

    printf("Main continues...\n");

    sleep(3);
}
// gcc pthread_detach.c -o a -pthread && ./a && rm a