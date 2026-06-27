#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>

/*
 * A tiny HTTP backend server, for the proxy/round-robin/health lessons.
 *
 * Usage:  ./backend <port> "<name>"
 * Example: ./backend 8081 "I am backend A"
 *
 * It answers:
 *   GET /        -> 200, body = "<name>"
 *   GET /health  -> 200, body = "OK"      (used by Lesson 6 health checks)
 *
 * Run several on different ports to simulate a pool of servers.
 */

static const char *g_name = "backend";

static void handle(int conn)
{
    char buffer[2048] = {0};
    if (read(conn, buffer, sizeof(buffer) - 1) <= 0) return;

    char method[16] = {0}, path[1024] = {0};
    sscanf(buffer, "%15s %1023s", method, path);
    printf("[%s] %s %s\n", g_name, method, path);
    fflush(stdout);

    char body[1100];
    if (strcmp(path, "/health") == 0)
        snprintf(body, sizeof(body), "OK");
    else
        snprintf(body, sizeof(body), "%s", g_name);

    char resp[1400];
    int len = snprintf(resp, sizeof(resp),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s", strlen(body), body);
    send(conn, resp, len, 0);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s <port> \"<name>\"\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    g_name = argv[2];

    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }
    listen(fd, 10);
    printf("backend \"%s\" listening on port %d\n", g_name, port);

    while (1) {
        int conn = accept(fd, NULL, NULL);
        if (conn < 0) continue;
        if (fork() == 0) {
            close(fd);
            handle(conn);
            close(conn);
            exit(0);
        }
        close(conn);
    }
    return 0;
}
