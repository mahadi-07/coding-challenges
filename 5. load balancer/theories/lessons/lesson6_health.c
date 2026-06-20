#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>

/*
 * LESSON 6: Health checks (challenge Step 3).  Compile with -lpthread.
 *
 * New vs Lesson 5:
 *  - We use THREADS (shared memory) so a background health thread can flip a
 *    `healthy` flag that the request path reads.
 *  - A mutex guards the shared backend state.
 *  - Round-robin SKIPS unhealthy backends.
 *
 * Usage: ./lesson6_health [health_period_seconds]   (default 2)
 */

#define LISTEN_PORT 8080

typedef struct {
    const char *host;
    int port;
    int healthy;     // shared: written by health thread, read by picker
} Backend;

static Backend backends[] = {
    {"127.0.0.1", 8081, 1},
    {"127.0.0.1", 8082, 1},
    {"127.0.0.1", 8083, 1},
};
static const int N = sizeof(backends) / sizeof(backends[0]);

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int rr = 0;   // round-robin cursor (guarded by lock)

/* ---- small helpers ---- */

static int dial(const char *host, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    struct sockaddr_in a = {0};
    a.sin_family = AF_INET;
    a.sin_port = htons(port);
    inet_pton(AF_INET, host, &a.sin_addr);
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

/* ---- health check: returns 1 if GET /health -> "200" ---- */

static int check_health(const char *host, int port)
{
    int fd = dial(host, port);
    if (fd < 0) return 0;

    char req[256];
    int rl = snprintf(req, sizeof(req),
        "GET /health HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", host);
    send(fd, req, rl, 0);

    char buf[256] = {0};
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) return 0;

    // crude but enough: look for "200" in the status line
    return strstr(buf, " 200 ") != NULL;
}

static void *health_loop(void *arg)
{
    int period = *(int *)arg;
    while (1) {
        for (int i = 0; i < N; i++) {
            int up = check_health(backends[i].host, backends[i].port);
            pthread_mutex_lock(&lock);
            if (backends[i].healthy != up)
                printf("[health] %s:%d -> %s\n", backends[i].host,
                       backends[i].port, up ? "UP" : "DOWN");
            backends[i].healthy = up;
            pthread_mutex_unlock(&lock);
        }
        fflush(stdout);
        sleep(period);
    }
    return NULL;
}

/* pick next HEALTHY backend, skipping dead ones; -1 if none */
static int pick_healthy(void)
{
    int idx = -1;
    pthread_mutex_lock(&lock);
    for (int tries = 0; tries < N; tries++) {
        int cand = rr;
        rr = (rr + 1) % N;
        if (backends[cand].healthy) { idx = cand; break; }
    }
    pthread_mutex_unlock(&lock);
    return idx;
}

static void send_502(int conn)
{
    const char *m = "HTTP/1.1 502 Bad Gateway\r\nContent-Length: 16\r\n"
                    "Connection: close\r\n\r\n502 Bad Gateway\n";
    send(conn, m, strlen(m), 0);
}

/* per-client worker (runs in its own thread) */
static void *handle(void *arg)
{
    int conn = *(int *)arg;
    free(arg);

    char req[8192] = {0};
    ssize_t reqlen = read(conn, req, sizeof(req) - 1);
    if (reqlen <= 0) { close(conn); return NULL; }

    int idx = pick_healthy();
    if (idx < 0) {
        printf("[lb] no healthy backends -> 502\n"); fflush(stdout);
        send_502(conn); close(conn); return NULL;
    }

    char method[16] = {0}, path[1024] = {0};
    sscanf(req, "%15s %1023s", method, path);
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

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    int period = (argc > 1) ? atoi(argv[1]) : 2;

    // start the background health-check thread
    pthread_t hth;
    pthread_create(&hth, NULL, health_loop, &period);
    pthread_detach(hth);

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
    listen(server_fd, 16);
    printf("health-aware LB on %d, health period = %ds\n", LISTEN_PORT, period);

    while (1) {
        int conn = accept(server_fd, NULL, NULL);
        if (conn < 0) continue;

        int *arg = malloc(sizeof(int));
        *arg = conn;
        pthread_t t;
        if (pthread_create(&t, NULL, handle, arg) != 0) {
            free(arg); close(conn); continue;
        }
        pthread_detach(t);   // auto-clean when the worker finishes
    }
    return 0;
}
