#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>

/*
 * LESSON 2: Concurrency with fork() — one process per client.
 *
 * Difference from lesson1_loop.c: after accept(), we fork() a child to
 * handle the client while the parent immediately loops back to accept().
 * A slow client no longer blocks the others.
 *
 * The handle() function sleeps 3 seconds on purpose to SIMULATE slow work,
 * so you can prove two clients are served at the same time.
 */

#define PORT 8080

static void handle(int conn, struct sockaddr_in *cli)
{
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cli->sin_addr, ip, sizeof(ip));
    printf("[pid %d] handling %s:%d ... (pretending to work for 3s)\n",
           getpid(), ip, ntohs(cli->sin_port));
    fflush(stdout);

    char buffer[1024] = {0};
    read(conn, buffer, sizeof(buffer) - 1);

    sleep(3);  // SIMULATE slow work so concurrency is visible

    const char *reply = "Hello from a concurrent worker\n";
    send(conn, reply, strlen(reply), 0);
    printf("[pid %d] done\n", getpid());
    fflush(stdout);
}

int main(void)
{
    // Auto-reap children so they don't become zombies. Try commenting this
    // out and watching `ps` fill with <defunct> processes.
    signal(SIGCHLD, SIG_IGN);

    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket"); exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind"); exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }

    printf("Concurrent server on port %d (Ctrl+C to stop)\n", PORT);

    while (1) {
        struct sockaddr_in cli;
        socklen_t clilen = sizeof(cli);
        int conn = accept(server_fd, (struct sockaddr *)&cli, &clilen);
        if (conn < 0) { perror("accept"); continue; }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(conn);
        } else if (pid == 0) {
            // CHILD: handle this one client, then exit
            close(server_fd);          // child doesn't need the listener
            handle(conn, &cli);
            close(conn);
            exit(0);
        } else {
            // PARENT: doesn't need the client socket; keep accepting
            close(conn);
        }
    }

    close(server_fd);
    return 0;
}
