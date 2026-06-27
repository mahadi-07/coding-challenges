#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>

/*
 * LESSON 4: Forwarding / proxy. With ONE backend, this is challenge Step 1.
 *
 * Be a SERVER to the client and a CLIENT to the backend:
 *   client -> [proxy] -> backend ;  backend -> [proxy] -> client
 */

#define LISTEN_PORT   8080
#define BACKEND_HOST  "127.0.0.1"
#define BACKEND_PORT  8081

/* client role: open a connection to the backend, return its fd (or -1) */
static int connect_to_backend(const char *host, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) { close(fd); return -1; }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;  // backend down / refused
    }
    return fd;
}

/* copy everything from src to dst until src hits EOF (with full-write loop) */
static void pipe_until_eof(int src, int dst)
{
    char buf[4096];
    ssize_t n;
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
    const char *msg =
        "HTTP/1.1 502 Bad Gateway\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 16\r\n"
        "Connection: close\r\n"
        "\r\n"
        "502 Bad Gateway\n";
    send(conn, msg, strlen(msg), 0);
}

static void handle(int conn)
{
    // 1. read the client's request
    char req[8192] = {0};
    ssize_t reqlen = read(conn, req, sizeof(req) - 1);
    if (reqlen <= 0) return;

    char method[16] = {0}, path[1024] = {0};
    sscanf(req, "%15s %1023s", method, path);
    printf("[proxy pid %d] %s %s -> %s:%d\n",
           getpid(), method, path, BACKEND_HOST, BACKEND_PORT);
    fflush(stdout);

    // 2. connect to the backend
    int back = connect_to_backend(BACKEND_HOST, BACKEND_PORT);
    if (back < 0) {
        printf("[proxy] backend unreachable -> 502\n");
        send_502(conn);
        return;
    }

    // 3. forward the request to the backend (full-write loop)
    ssize_t off = 0;
    while (off < reqlen) {
        ssize_t w = send(back, req + off, reqlen - off, 0);
        if (w <= 0) { close(back); return; }
        off += w;
    }

    // 4. relay the backend's reply back to the client
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
    printf("proxy listening on %d, forwarding to %s:%d\n",
           LISTEN_PORT, BACKEND_HOST, BACKEND_PORT);

    while (1) {
        int conn = accept(server_fd, NULL, NULL);
        if (conn < 0) continue;
        if (fork() == 0) {
            close(server_fd);
            handle(conn);
            close(conn);
            exit(0);
        }
        close(conn);
    }
    return 0;
}
