#include <stdio.h>
#include <pthread.h>

void *thread_func(void *arg)
{
    int value = *(int *) arg;
    printf("Received: %d\n", value);

    return NULL;
}

int main()
{
    pthread_t tid;
    int x = 10;
    pthread_create(&tid, NULL, thread_func, &x);
    pthread_join(tid, NULL);
    return 0;
}