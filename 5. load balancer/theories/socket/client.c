#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

void *worker(void *arg)
{
    int id = *(int *)arg;

    int sock;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sock,
            (struct sockaddr *)&serv_addr,
            sizeof(serv_addr));

    char msg[64];
    sprintf(msg, "Hello from thread %d", id);

    send(sock, msg, strlen(msg), 0);

    read(sock, buffer, sizeof(buffer));

    printf("[Thread %d] Response: %s\n", id, buffer);

    close(sock);

    return NULL;
}

int main(void)
{
    int n = 100;
    pthread_t threads[n];
    int ids[n];

    for (int i = 0; i < n; i++) {
        ids[i] = i;

        pthread_create(
            &threads[i],
            NULL,
            worker,
            &ids[i]
        );
    }

    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

// gcc client.c -o client -lpthread && time ./client