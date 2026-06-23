#include <stdio.h>
#include <pthread.h>

#define TIMES_TO_COUNT 21000

#define NC "\e[0m"
#define YELLOW "\e[33m"
#define BYELLOW "\e[1;33m"
#define RED "\e[31m"
#define GREEN "\e[32m"

void *thread_routine(void *data)
{
    pthread_t tid;
    unsigned int *count;
    unsigned int i;

    tid = pthread_self();
    count = (unsigned int *) data;
    printf("%sThread [%p]: Count at thread start = %u.%s\n", YELLOW, tid, *count, NC);

    i = 0;
    while(i < TIMES_TO_COUNT)
    {
        (*count)++;
        i++;
    }

    printf("%sThread [%p]: Final count = %u.%s\n", BYELLOW, tid, *count, NC);
    return NULL;
}

int main()
{
    pthread_t tid1;
    pthread_t tid2;

    unsigned int count;

    count = 0;

    printf("Main: Expected count is %s%u%s\n", GREEN, 2 * TIMES_TO_COUNT, NC);

    pthread_create(&tid1, NULL, thread_routine, &count);
    printf("Main: Created first thread [%p]\n", tid1);
    pthread_create(&tid2, NULL, thread_routine, &count);
    printf("Main: Created second thread [%p]\n", tid2);

    pthread_join(tid1, NULL);
    printf("Main: Joined first thread [%p]\n", tid1);
    pthread_join(tid2, NULL);
    printf("Main: Joined second thread [%p]\n", tid2);

    if(count != (2 * TIMES_TO_COUNT))
        printf("%sMain: ERROR ! Total count is %u%s\n", RED, count, NC);
    else
        printf("%sMain: OK. Total count is %u%s\n", GREEN, count, NC);

    return 0;
}
// gcc threads_shared_memory.c -o a -pthread && ./a && rm a
// gcc threads_shared_memory.c -o a -pthread && for i in {1..20}; do ./a | tail -n1; done; rm -f a
// gcc -fsanitize=thread -g threads_shared_memory.c -o a -pthread && ./a && rm a