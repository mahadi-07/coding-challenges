#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct ThreadData T_data;
struct ThreadData {
    int thread_id;
    char *message;
};

void *print_message(void *arg)
{
    T_data *data = (T_data *) arg;

    unsigned int seed = (unsigned int)time(NULL) ^ data->thread_id;
    int delay = rand_r(&seed) % 10 + 1;
    sleep(delay);

    printf("Thread %d says: %s\n", data->thread_id, data->message);

    free(data);

    pthread_exit(NULL);
}

int main()
{
    int n = 100;
    pthread_t tids[n + 1];
    for(int i = 1; i <= n; i++) {
        T_data *args = malloc(sizeof(T_data));
        args->thread_id = i;
        args->message = "Hello from the worker thread!";

        int result = pthread_create(&tids[i], NULL, print_message, (void *)args);

        if(result != 0) {
            perror("Failed to created thread");
            free(args);
            return 1;
        }
    }

    for(int i = 1; i <= n; i++) {
        pthread_join(tids[i], NULL);
    }

    return 0;
}