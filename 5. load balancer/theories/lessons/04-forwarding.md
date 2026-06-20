# Lesson 4 — Forwarding / proxying (the core LB move)

**Code:** `lessons/lesson4_proxy.c`  +  `lessons/backend.c`

## The flaw it fixes
Lesson 3's server answers requests *itself*. A load balancer answers nothing
— it **forwards** the client's request to a backend and relays the reply.
**This lesson, with ONE backend, IS challenge Step 1.**

## The big idea: be a server AND a client at once
```
client ──(1)──► [ PROXY ] ──(2)──► backend
       ◄─(4)──           ◄─(3)──
```
1. Accept the client (server role — what we've done all along).
2. `connect()` to the backend (client role — like `socket/client.c`!).
3. Read the backend's response.
4. Write that response back to the client.

So the proxy reuses BOTH halves you already know: the server setup from
Lessons 1–3, and the `socket`+`connect` from `client.c`.

## Connecting to the backend (client role)
Factor it into a helper that returns a connected socket fd:
```c
int connect_to_backend(const char *host, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd); return -1;          // backend down
    }
    return fd;
}
```

## Piping bytes (and a real-world gotcha)
`read()`/`send()` may move FEWER bytes than asked. Always loop. To copy
everything from one fd to another until EOF:
```c
char buf[4096]; ssize_t n;
while ((n = read(src, buf, sizeof(buf))) > 0) {
    ssize_t off = 0;
    while (off < n) {                    // full-write loop
        ssize_t w = send(dst, buf + off, n - off, 0);
        if (w <= 0) return;
        off += w;
    }
}
```

## SIGPIPE
If you `send()` to a connection the other end already closed, the OS raises
`SIGPIPE` which kills your process by default. Ignore it once at startup:
```c
signal(SIGPIPE, SIG_IGN);
```

## The flow inside the child (per client)
```c
int back = connect_to_backend("127.0.0.1", 8081);
if (back < 0) { /* send 502 Bad Gateway to client */ }
copy(conn, back);   // client request  -> backend
copy(back, conn);   // backend reply   -> client
close(back);
```
(Reading the whole request then the whole reply is fine for simple GETs.
A general proxy would pump both directions at once with select/poll — beyond
Step 1.)

## Run it (you need 3 terminals OR background jobs)
```sh
# 1. a backend on 8081 (our tiny HTTP server)
cc lessons/backend.c -o /tmp/backend
/tmp/backend 8081 "I am backend A" &

# 2. the proxy on 8080, forwarding to 8081
cc lessons/lesson4_proxy.c -o /tmp/proxy
/tmp/proxy &

# 3. hit the PROXY; the BACKEND's body comes back
curl http://localhost:8080/
# -> "I am backend A"
kill %1 %2
```

## The flaw that motivates Lesson 5
One hard-coded backend isn't "balancing." Next: a list of backends and
round-robin selection.

## Exercise
Stop the backend, then `curl` the proxy. Make the proxy detect the failed
`connect()` and return `HTTP/1.1 502 Bad Gateway` instead of hanging.
