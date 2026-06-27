# Lesson 7 — Assemble the final `lb` (the challenge)

**Code:** `lessons/lb.c`  (compile with `-lpthread`)

## What this is
Lesson 6 already has every behavior the challenge needs (concurrency,
forwarding, round-robin, health checks). The only thing left is to make it
**configurable from the command line** instead of hard-coding ports, and to
tidy the logging. `lb.c` does that.

## Maps to the challenge
- Step 1: listens, logs requests, forwards, returns response, concurrent ✅
- Step 2: multiple backends, round-robin ✅
- Step 3: periodic health checks, drop/re-add, configurable period ✅

## Configuration (CLI)
```
./lb [options]
  -p <port>            port to listen on            (default 80, use 8080 unprivileged)
  -b <host:port>       add a backend (repeatable)
  -h <path>            health-check path            (default /health)
  -t <seconds>         health-check period          (default 10)
```
Example:
```sh
./lb -p 8080 -b 127.0.0.1:8081 -b 127.0.0.1:8082 -b 127.0.0.1:8083 -h /health -t 5
```
Parsed with `getopt`. Backends are added to a dynamic array as `-b` repeats.

## End-to-end test (the demo the challenge expects)
```sh
cc lessons/backend.c -o /tmp/backend
cc lessons/lb.c -o /tmp/lb -lpthread

/tmp/backend 8081 "server-1" &
/tmp/backend 8082 "server-2" &
/tmp/backend 8083 "server-3" &

/tmp/lb -p 8080 -b 127.0.0.1:8081 -b 127.0.0.1:8082 -b 127.0.0.1:8083 -t 3 &

# round-robin
for i in $(seq 6); do curl -s localhost:8080/; echo; done   # 1 2 3 1 2 3

# health: kill one, watch it drop, restart, watch it return
kill %2; sleep 4
for i in $(seq 6); do curl -s localhost:8080/; echo; done   # only 2 and 3...
```

## Things to polish "beyond Step 3" (optional)
- HTTP keep-alive (reuse backend connections) for speed.
- Proper request/response streaming both directions with poll()/select()
  (handles large bodies, slow clients).
- Weighted or least-connections balancing instead of plain round-robin.
- Structured access logs (status code, bytes, latency).
- Graceful shutdown on SIGINT.

## You're done
At this point `lb.c` satisfies Steps 1–3. Port the ideas to your language of
choice if you prefer; the concepts (accept loop, concurrency, forward, rotate,
health-check thread + mutex) are identical everywhere.
