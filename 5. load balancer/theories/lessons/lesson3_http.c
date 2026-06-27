#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>

/*
 * LESSON 3: Speak HTTP.
 *
 * Difference from lesson2_fork.c: instead of a custom string, we parse the
 * HTTP request line (METHOD PATH VERSION) for logging, and we send back a
 * proper HTTP/1.1 response with a correct Content-Length so curl/browsers
 * are happy. (Still concurrent via fork, carried over from Lesson 2.)
 */

#define PORT 8080

static void handle(int conn)
{
    char buffer[4096] = {0};
    ssize_t n = read(conn, buffer, sizeof(buffer) - 1);
    if (n <= 0) return;

    // Parse the first line: e.g. "GET /hello HTTP/1.1"
    char method[16] = {0}, path[1024] = {0}, version[16] = {0};
    sscanf(buffer, "%15s %1023s %15s", method, path, version);
    printf("[pid %d] %s %s %s\n", getpid(), method, path, version);
    fflush(stdout);

    // Build a valid HTTP response. Content-Length MUST match body length.
    const char *body = "Hello, World! (from the HTTP lesson)\n";
    char resp[512];
    int len = snprintf(resp, sizeof(resp),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s", strlen(body), body);

    send(conn, resp, len, 0);
}

int main(void)
{
    signal(SIGCHLD, SIG_IGN);   // auto-reap children (Lesson 2)

    int server_fd;
    struct sockaddr_in address;

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

    printf("HTTP server on http://localhost:%d (Ctrl+C to stop)\n", PORT);

    while (1) {
        struct sockaddr_in cli;
        socklen_t clilen = sizeof(cli);
        int conn = accept(server_fd, (struct sockaddr *)&cli, &clilen);
        if (conn < 0) { perror("accept"); continue; }

        pid_t pid = fork();
        if (pid == 0) {
            close(server_fd);
            handle(conn);
            close(conn);
            exit(0);
        }
        close(conn);
    }
    close(server_fd);
    return 0;
}
