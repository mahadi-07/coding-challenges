# Lesson 2 — Concurrency (serve clients at the same time)

**Code:** `lessons/lesson2_fork.c`

## The flaw it fixes
Lesson 1's loop is sequential: one slow client blocks ALL others, because
the single process is stuck in `read()`. Challenge Step 1 *requires*
concurrency.

## The idea: one worker per client
After `accept()`, hand the client off to a **separate worker** so the main
loop can immediately `accept()` the next one. Two common ways:

### A) `fork()` — one PROCESS per client (this lesson's main demo)
`fork()` clones the process. It returns:
- `0` in the **child**  → child handles the client, then `exit`.
- child's PID in the **parent** → parent closes `conn` and loops to accept.
- `-1` on error.

```c
int conn = accept(...);
pid_t pid = fork();
if (pid == 0) {            // CHILD
    close(server_fd);      // child doesn't need the listener
    handle(conn);
    close(conn);
    exit(0);
} else {                   // PARENT
    close(conn);           // parent doesn't need the client socket
    // loop back to accept()
}
```

**Why both sides `close()`:** after `fork()` the fd is open in BOTH
processes. Each closes the copy it doesn't use, or the connection never
fully closes.

### Zombies (important!)
When a child exits, it stays a "zombie" until the parent reaps it. Ignore
this and you leak process slots. Fix: tell the OS to auto-reap.
```c
signal(SIGCHLD, SIG_IGN);   // auto-reap dead children
```
(Alternative: a `SIGCHLD` handler that calls `waitpid(-1, NULL, WNOHANG)`.)

### B) `pthread` — one THREAD per client (lighter; alternative)
Threads share memory (cheaper than fork). Needs `-lpthread`.
```c
pthread_t t;
int *arg = malloc(sizeof(int)); *arg = conn;
pthread_create(&t, NULL, worker, arg);
pthread_detach(t);          // auto-clean when it finishes
```
We'll *use threads* for real in Lesson 6 (health-check background thread).
For Step 1, fork is simplest; pick either.

## Prove it works: the slow-client test
The demo sleeps 3s while "handling" a client. With concurrency, a second
client connecting during that sleep is served **immediately**, not after 3s.

```sh
cc lessons/lesson2_fork.c -o /tmp/fork
/tmp/fork &
# fire two clients at once; both should finish ~together, not 3s apart
time (printf "A" | nc -w5 127.0.0.1 8080) &
time (printf "B" | nc -w5 127.0.0.1 8080) &
wait
kill %1
```

## The flaw that motivates Lesson 3
We can now serve many clients at once — but we only speak a custom string.
Browsers and `curl` speak **HTTP**. Next lesson: speak HTTP.

## Exercise
Comment out `signal(SIGCHLD, SIG_IGN);`, run many clients, then run
`ps aux | grep -c defunct` — watch zombies pile up. Re-enable it; they vanish.
