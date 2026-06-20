# Lesson 0 — Prerequisites & vocabulary

These are *ideas*, not code to run. You already met most in `socket/`.

## A socket is a file descriptor
A socket is just an `int` the OS hands you. It represents one end of a
connection. You read/write it much like a file.

## Client–server model
One side **listens and waits** (server). The other **initiates** (client).
A load balancer is BOTH at once.

## TCP vs UDP
- `SOCK_STREAM` = **TCP**: reliable, ordered, connection-based. ← we use this.
- `SOCK_DGRAM`  = **UDP**: fast, unordered, no connection. (not used here)

## IP address + port
- IP address → which machine (`127.0.0.1` = this machine, "localhost").
- Port → which program on that machine (e.g. `8080`).

## Byte order (htons / htonl)
Networks agree on **big-endian** ("network byte order"). Your CPU may be
little-endian. Convert numbers before putting them on the wire:
- `htons` = host-to-network short  (for ports)
- `htonl` = host-to-network long   (for IPv4 addresses)
- `ntohs` / `ntohl` = the reverse.

## Blocking calls
`accept()` and `read()` **pause your program** until something happens
(a client connects / data arrives). This single fact is why we need
concurrency (Lesson 2).

## The 7 core syscalls

| Call        | Side   | Meaning                                  |
|-------------|--------|------------------------------------------|
| `socket()`  | both   | create the endpoint (get an fd)          |
| `bind()`    | server | claim an IP+port                         |
| `listen()`  | server | mark socket as passive; set backlog      |
| `accept()`  | server | wait for a client; get a NEW socket      |
| `connect()` | client | dial a server                            |
| `send()`/`write()` | both | send bytes                         |
| `recv()`/`read()`  | both | receive bytes                      |
| `close()`   | both   | hang up                                  |

Server order:  socket → bind → listen → accept → read/send → close
Client order:  socket → connect → send/read → close

See `socket/server.c` and `socket/client.c` for the working code.
