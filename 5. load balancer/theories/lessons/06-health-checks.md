# Lesson 6 — Health checks (Step 3)

**Code:** `lessons/lesson6_health.c`  (compile with `-lpthread`)

## The flaw it fixes
Round-robin happily forwards to a dead backend → clients get errors. Step 3:
periodically check each backend; only route to healthy ones; re-add them when
they recover.

## What "healthy" means
A background task sends `GET /health` to each backend every N seconds.
- Replies `HTTP/1.1 200` → **healthy**.
- Refused / timeout / non-200 → **unhealthy** → drop from rotation.
Recovers later → mark healthy → back in rotation.

## Why this lesson switches to THREADS
We need:
1. a **background worker** that loops forever doing health checks, AND
2. **shared state** (each backend's `healthy` flag) that both the health
   thread (writer) and the request handlers (readers) can see.

`fork()` children have separate memory, so a forked child can't see a flag
the health process flipped. **Threads share memory** → perfect fit. We'll use
one thread for health checks and threads (or fork) for clients.

```c
typedef struct {
    const char *host; int port;
    int healthy;            // shared flag
} Backend;
```

## Mutex: don't read a flag while it's being written
Multiple threads touching `healthy` at once = data race. Guard it:
```c
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_lock(&lock);
backends[i].healthy = up ? 1 : 0;
pthread_mutex_unlock(&lock);
```
Lock around BOTH the health thread's writes and the picker's reads.

## Round-robin that SKIPS unhealthy servers
```c
int pick_healthy(void) {
    pthread_mutex_lock(&lock);
    for (int tries = 0; tries < N; tries++) {
        int idx = rr;
        rr = (rr + 1) % N;
        if (backends[idx].healthy) { pthread_mutex_unlock(&lock); return idx; }
    }
    pthread_mutex_unlock(&lock);
    return -1;   // none healthy
}
```

## The health-check thread
```c
void *health_loop(void *arg) {
    int period = *(int*)arg;
    while (1) {
        for (int i = 0; i < N; i++) {
            int up = http_get_returns_200(backends[i].host, backends[i].port, "/health");
            pthread_mutex_lock(&lock);
            backends[i].healthy = up;
            pthread_mutex_unlock(&lock);
        }
        sleep(period);
    }
    return NULL;
}
// start it once:
pthread_t th; pthread_create(&th, NULL, health_loop, &period); pthread_detach(th);
```

## Config via command line (Step 3 asks for this)
e.g. `./lb --health-period 5`. Lesson keeps it simple: read `argv[1]` as the
period in seconds. The final `lb` (Lesson 7) parses more flags.

## Run it
```sh
cc lessons/backend.c -o /tmp/backend
/tmp/backend 8081 "A" &
/tmp/backend 8082 "B" &
/tmp/backend 8083 "C" &

cc lessons/lesson6_health.c -o /tmp/health -lpthread
/tmp/health 2 &        # health-check every 2s

for i in $(seq 6); do curl -s localhost:8080/; echo; done   # A B C A B C
kill %2                                                       # kill backend B
sleep 3                                                       # let a health pass run
for i in $(seq 6); do curl -s localhost:8080/; echo; done   # now only A C A C...
kill %1 %3 %4
```

## The flaw that motivates Lesson 7
We now have all the pieces in separate demo files. Lesson 7 merges them into
ONE configurable `lb` program (port, backend list, health URL, period).

## Exercise
After killing backend B and seeing it drop out, RESTART it
(`/tmp/backend 8082 "B" &`), wait one health period, and confirm B rejoins
the rotation automatically.
