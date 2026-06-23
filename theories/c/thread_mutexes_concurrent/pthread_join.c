#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define NC "\e[0m"
#define YELLOW "\e[1;33m"

void *thread_routine(void *data)
{
    pthread_t tid;
    tid = pthread_self();
    printf("%sThread [%p]: The heaviest burden is to exist without living.%s\n",
        YELLOW, tid, NC);

    return NULL;
}

int main()
{
    pthread_t tid1;
    pthread_t tid2;

    pthread_create(&tid1, NULL, thread_routine, NULL);
    printf("Main: Create first thread [%p]\n", tid1);

    sleep(3);

    pthread_create(&tid2, NULL, thread_routine, NULL);
    printf("Main: Created second thread [%p]\n", tid2);

    sleep(3);

    pthread_join(tid1, NULL);
    printf("Main: Joining first thread [%p]\n", tid1);

    sleep(10);

    pthread_join(tid2, NULL);
    printf("Main: Joining second thread [%p]\n", tid2);

    return 0;
}