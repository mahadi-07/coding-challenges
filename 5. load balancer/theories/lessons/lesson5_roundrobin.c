#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>

/*
 * LESSON 5: Round-robin across multiple backends (challenge Step 2).
 *
 * Key idea: the PARENT process picks the next backend (its counter advances
 * correctly because it's one process), THEN forks a child to do the forward.
 * If we incremented the counter inside the child, every child would have its
 * own copy and the rotation would break — see 05-round-robin.md.
 */

#define LISTEN_PORT 8080

typedef struct { const char *host; int port; } Backend;

static Backend backends[] = {
    {"127.0.0.1", 8081},
    {"127.0.0.1", 8082},
    {"127.0.0.1", 8083},
};
static const int N = sizeof(backends) / sizeof(backends[0]);

/* advance the rotation in the parent; return the chosen index */
static int next_backend(void)
{
    static int i = 0;
    int chosen = i;
    i = (i + 1) % N;
    return chosen;
}

static int connect_to_backend(const Backend *b)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(b->port);
    inet_pton(AF_INET, b->host, &addr.sin_addr);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd); return -1;
    }
    return fd;
}

static void pipe_until_eof(int src, int dst)
{
    char buf[4096]; ssize_t n;
    while ((n = read(src, buf, sizeof(buf))) > 0) {
        ssize_t off = 0;
        while (off < n) {
            ssize_t w = send(dst, buf + off, n - off, 0);
            if (w <= 0) return;
            off += w;
        }
    }
}

static void send_502(int conn)
{
    const char *m = "HTTP/1.1 502 Bad Gateway\r\nContent-Length: 16\r\n"
                    "Connection: close\r\n\r\n502 Bad Gateway\n";
    send(conn, m, strlen(m), 0);
}

static void handle(int conn, const Backend *b)
{
    char req[8192] = {0};
    ssize_t reqlen = read(conn, req, sizeof(req) - 1);
    if (reqlen <= 0) return;

    char method[16] = {0}, path[1024] = {0};
    sscanf(req, "%15s %1023s", method, path);
    printf("[lb pid %d] %s %s -> %s:%d\n",
           getpid(), method, path, b->host, b->port);
    fflush(stdout);

    int back = connect_to_backend(b);
    if (back < 0) { send_502(conn); return; }

    ssize_t off = 0;
    while (off < reqlen) {
        ssize_t w = send(back, req + off, reqlen - off, 0);
        if (w <= 0) { close(back); return; }
        off += w;
    }
    pipe_until_eof(back, conn);
    close(back);
}

int main(void)
{
    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(LISTEN_PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(EXIT_FAILURE);
    }
    listen(server_fd, 10);
    printf("round-robin LB on %d across %d backends\n", LISTEN_PORT, N);

    while (1) {
        int conn = accept(server_fd, NULL, NULL);
        if (conn < 0) continue;

        int idx = next_backend();        // PARENT advances rotation
        if (fork() == 0) {
            close(server_fd);
            handle(conn, &backends[idx]);
            close(conn);
            exit(0);
        }
        close(conn);
    }
    return 0;
}
