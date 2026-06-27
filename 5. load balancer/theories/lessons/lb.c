#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>

/*
 * lb.c — a small load balancer satisfying challenge Steps 1–3.
 * Compile:  cc lb.c -o lb -lpthread
 *
 * Options:
 *   -p <port>        listen port              (default 8080)
 *   -b <host:port>   add a backend (repeat)   (at least one required)
 *   -h <path>        health check path        (default /health)
 *   -t <seconds>     health check period      (default 10)
 *
 * Example:
 *   ./lb -p 8080 -b 127.0.0.1:8081 -b 127.0.0.1:8082 -t 5
 */

#define MAX_BACKENDS 64

typedef struct {
    char host[64];
    int  port;
    int  healthy;
} Backend;

static Backend backends[MAX_BACKENDS];
static int     n_backends = 0;

static char health_path[256] = "/health";
static int  health_period    = 10;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int rr = 0;

/* ---------- helpers ---------- */

static int dial(const char *host, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    struct sockaddr_in a = {0};
    a.sin_family = AF_INET;
    a.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &a.sin_addr) <= 0) { close(fd); return -1; }
    if (connect(fd, (struct sockaddr *)&a, sizeof(a)) < 0) { close(fd); return -1; }
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

/* ---------- health checking ---------- */

static int check_health(const Backend *b)
{
    int fd = dial(b->host, b->port);
    if (fd < 0) return 0;
    char req[512];
    int rl = snprintf(req, sizeof(req),
        "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
        health_path, b->host);
    send(fd, req, rl, 0);
    char buf[256] = {0};
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    return (n > 0) && (strstr(buf, " 200 ") != NULL);
}

static void *health_loop(void *unused)
{
    (void)unused;
    while (1) {
        for (int i = 0; i < n_backends; i++) {
            int up = check_health(&backends[i]);
            pthread_mutex_lock(&lock);
            if (backends[i].healthy != up)
                printf("[health] %s:%d -> %s\n",
                       backends[i].host, backends[i].port, up ? "UP" : "DOWN");
            backends[i].healthy = up;
            pthread_mutex_unlock(&lock);
        }
        fflush(stdout);
        sleep(health_period);
    }
    return NULL;
}

/* round-robin over healthy backends; returns index or -1 */
static int pick_healthy(void)
{
    int idx = -1;
    pthread_mutex_lock(&lock);
    for (int t = 0; t < n_backends; t++) {
        int c = rr;
        rr = (rr + 1) % n_backends;
        if (backends[c].healthy) { idx = c; break; }
    }
    pthread_mutex_unlock(&lock);
    return idx;
}

/* ---------- per-client worker ---------- */

static void *handle(void *arg)
{
    int conn = *(int *)arg;
    free(arg);

    char req[8192] = {0};
    ssize_t reqlen = read(conn, req, sizeof(req) - 1);
    if (reqlen <= 0) { close(conn); return NULL; }

    char method[16] = {0}, path[1024] = {0};
    sscanf(req, "%15s %1023s", method, path);

    int idx = pick_healthy();
    if (idx < 0) {
        printf("[lb] %s %s -> NO HEALTHY BACKEND (502)\n", method, path);
        fflush(stdout);
        send_502(conn); close(conn); return NULL;
    }

    printf("[lb] %s %s -> %s:%d\n", method, path,
           backends[idx].host, backends[idx].port);
    fflush(stdout);

    int back = dial(backends[idx].host, backends[idx].port);
    if (back < 0) { send_502(conn); close(conn); return NULL; }

    ssize_t off = 0;
    while (off < reqlen) {
        ssize_t w = send(back, req + off, reqlen - off, 0);
        if (w <= 0) { close(back); close(conn); return NULL; }
        off += w;
    }
    pipe_until_eof(back, conn);
    close(back);
    close(conn);
    return NULL;
}

/* ---------- config parsing ---------- */

static void add_backend(const char *hostport)
{
    if (n_backends >= MAX_BACKENDS) {
        fprintf(stderr, "too many backends\n");
        return;
    }
    const char *colon = strrchr(hostport, ':');
    if (!colon) { fprintf(stderr, "bad backend '%s' (need host:port)\n", hostport); return; }
    size_t hlen = (size_t)(colon - hostport);
    if (hlen >= sizeof(backends[0].host)) hlen = sizeof(backends[0].host) - 1;
    memcpy(backends[n_backends].host, hostport, hlen);
    backends[n_backends].host[hlen] = '\0';
    backends[n_backends].port = atoi(colon + 1);
    backends[n_backends].healthy = 1;   // assume up until first health pass
    n_backends++;
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);

    int listen_port = 8080;
    int opt;
    while ((opt = getopt(argc, argv, "p:b:h:t:")) != -1) {
        switch (opt) {
            case 'p': listen_port = atoi(optarg); break;
            case 'b': add_backend(optarg); break;
            case 'h': snprintf(health_path, sizeof(health_path), "%s", optarg); break;
            case 't': health_period = atoi(optarg); break;
            default:
                fprintf(stderr,
                  "usage: %s -p port -b host:port [-b host:port ...] "
                  "[-h /health] [-t seconds]\n", argv[0]);
                return 1;
        }
    }
    if (n_backends == 0) {
        fprintf(stderr, "error: provide at least one -b host:port\n");
        return 1;
    }

    pthread_t hth;
    pthread_create(&hth, NULL, health_loop, NULL);
    pthread_detach(hth);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int one = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(listen_port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }
    listen(server_fd, 32);

    printf("lb listening on :%d | %d backend(s) | health %s every %ds\n",
           listen_port, n_backends, health_path, health_period);

    while (1) {
        int conn = accept(server_fd, NULL, NULL);
        if (conn < 0) continue;
        int *a = malloc(sizeof(int));
        *a = conn;
        pthread_t t;
        if (pthread_create(&t, NULL, handle, a) != 0) {
            free(a); close(conn); continue;
        }
        pthread_detach(t);
    }
    return 0;
}
