#include <stdio.h>
#include <pthread.h>

#define TIMES_TO_COUNT 50000

#define NC "\e[0m"
#define YELLOW "\e[33m"
#define BYELLOW "\e[1;33m"
#define RED "\e[31m"
#define GREEN "\e[32m"

/**
 * This structuree contains the count as well as the mutex that will
 * protect the acess to the variable.
 */
typedef struct s_counter
{
    pthread_mutex_t count_mutex;
    unsigned int count;
} t_counter;

void *thread_routine(void *data)
{
    pthread_t tid;
    t_counter *counter;
    unsigned int i;

    tid = pthread_self();
    counter = (t_counter *) data;

    // Print the count before this thread starts iterating
    // In ordere to read thee value of count, we lock the mutex
    pthread_mutex_lock(&counter->count_mutex);
     printf("%sThread [%p]: Count at thread start = %u.%s\n",
        YELLOW, tid, counter->count, NC);
    pthread_mutex_unlock(&counter->count_mutex);

    i = 0;
    while(i < TIMES_TO_COUNT)
    {
        pthread_mutex_lock(&counter->count_mutex);
        counter->count++;
        pthread_mutex_unlock(&counter->count_mutex);
        i++;
    }

    pthread_mutex_lock(&counter->count_mutex);
    printf("%sThread [%p]: Final count = %u.%s\n",
        BYELLOW, tid, counter->count, NC);
    pthread_mutex_unlock(&counter->count_mutex);

    return NULL;
}

int main()
{
    pthread_t tid1;
    pthread_t tid2;

    // structure containing the threads' total count
    t_counter counter;

    // This is only one thread here (main thread), so we can safely
    // initialize count without using the mutex
    counter.count = 0;

    // initialize the mutex
    pthread_mutex_init(&counter.count_mutex, NULL);

    printf("Main: Expected count is %s%u%s\n", 
        GREEN, 2 * TIMES_TO_COUNT, NC);
    
    // Thread creation
    pthread_create(&tid1, NULL, thread_routine, &counter);
    printf("Main: Created first thread [%p]\n", tid1);
    pthread_create(&tid2, NULL, thread_routine, &counter);
    printf("Main: Created second thread [%p]\n", tid2);

    // Thread joining
    pthread_join(tid1, NULL);
    printf("Main: Joined first thread [%p]\n", tid1);
    pthread_join(tid2, NULL);
    printf("Main: Joined second thread [%p]\n", tid2);

    // Final count evaluation
    if(counter.count != (2*TIMES_TO_COUNT))
        printf("%sMain: ERROR ! Total count is %u%s\n",
            RED, counter.count, NC);
    else
        printf("%sMain: OK. Total count is %u%s\n",
            GREEN, counter.count, NC);

    pthread_mutex_destroy(&counter.count_mutex);

    return 0;
}