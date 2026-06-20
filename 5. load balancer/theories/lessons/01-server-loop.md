# Lesson 1 — The server loop

**Code:** `socket/lesson1_loop.c`  (already built & tested)

## The flaw it fixes
`socket/server.c` does: accept ONE client → reply → `return 0`. It dies.
A server must run forever.

## The idea
Wrap `accept()` in `while(1)`. `server_fd` keeps listening; each client gets
its own socket `conn`. Handle it, `close(conn)`, loop back.

```
create → bind → listen
   └─► while(1):
          conn = accept()      // new socket per client
          read / send on conn
          close(conn)          // close the CLIENT socket, NOT server_fd
```

## Three things introduced
1. **The loop** — the heart of every server.
2. **`SO_REUSEADDR`** — without it, restarting fast gives
   "Address already in use" (port stuck in TIME_WAIT ~60s).
   ```c
   int opt = 1;
   setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
   ```
3. **`inet_ntop`** — turn the client's binary address into a readable IP
   string (inverse of the client's `inet_pton`).

## Also: a real bug in server.c
`server.c` checks `if ((server_fd = socket(...)) == 0)`. Wrong — `socket()`
returns **-1** on failure. Correct check is `< 0`. (client.c got it right.)

## Run it
```sh
cc socket/lesson1_loop.c -o /tmp/loop
/tmp/loop &                       # start server
printf "hi"  | nc -w1 127.0.0.1 8080
printf "yo"  | nc -w1 127.0.0.1 8080   # SAME server answers again
kill %1
```

## The flaw that motivates Lesson 2
This loop is **sequential**. While `read()` waits on a slow client, every
other client is stuck in the backlog. Step 1 of the challenge *requires*
handling clients **simultaneously** → next lesson.

## Exercise
Add an `int count = 0;` before the loop, `count++` after each accept, and
print `Client #N`. Confirm the number rises across separate `nc` runs.
