# Lesson 5 — Round-robin across multiple backends (Step 2)

**Code:** `lessons/lesson5_roundrobin.c`  (uses `backend.c` from Lesson 4)

## The flaw it fixes
Lesson 4 forwards to ONE hard-coded backend. "Load balancing" means
spreading requests across MANY. Step 2: round-robin scheduling — send to
each backend in turn, then wrap back to the start.

## The idea
Keep a list of backends and a rotating index.
```c
typedef struct { char host[64]; int port; } Backend;
Backend backends[] = {
    {"127.0.0.1", 8081},
    {"127.0.0.1", 8082},
    {"127.0.0.1", 8083},
};
int n = 3;

int next_backend(void) {
    static int i = 0;
    int chosen = i;
    i = (i + 1) % n;     // wrap around: ...2,0,1,2,0,1...
    return chosen;
}
```
`(i + 1) % n` is the whole trick: after the last backend, modulo wraps the
index back to 0.

## ⚠️ The fork() + counter gotcha (important concept!)
With `fork()`, each child is a SEPARATE process with its OWN copy of memory.
A plain `static int i` does NOT increment across children — every child sees
its own copy. Round-robin then breaks.

Three ways to deal with it:
1. **Pick the backend in the PARENT, before forking**, and pass the index to
   the child. The parent is one process, so its counter advances correctly.
   ← simplest; this lesson does this.
2. Use **threads** instead of processes (shared memory) + a mutex.
3. Use **shared memory** (`mmap`/`shm`) for the counter across processes.

This is exactly why many people choose threads for the final `lb`. Keep it
in mind for Lesson 7.

## Flow (parent chooses, child forwards)
```c
int idx = next_backend();          // PARENT advances the rotation
int conn = accept(...);
if (fork() == 0) {                  // CHILD
    forward_to(backends[idx], conn);
    exit(0);
}
```

## Run it
```sh
cc lessons/backend.c -o /tmp/backend
/tmp/backend 8081 "backend-A" &
/tmp/backend 8082 "backend-B" &
/tmp/backend 8083 "backend-C" &

cc lessons/lesson5_roundrobin.c -o /tmp/rr
/tmp/rr &

for i in 1 2 3 4 5 6; do curl -s localhost:8080/; echo; done
# Expect: A, B, C, A, B, C  (rotation!)
kill %1 %2 %3 %4
```

## The flaw that motivates Lesson 6
If backend-B crashes, round-robin still sends every 2nd-in-3 request to a
dead server → errors. We need to detect dead servers and skip them. Next:
health checks.

## Exercise
Add a 4th backend on 8084 but DON'T start it. Run the curl loop. Watch every
4th request fail/502. That failure is the motivation for Lesson 6.
