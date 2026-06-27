# Load Balancer — Theory Course

Goal: by the end, solve <https://codingchallenges.fyi/challenges/challenge-load-balancer>.

A load balancer is a box in the middle. It **receives** requests like a server,
and **forwards** them to one of several backends like a client, then passes the
reply back.

```
   client  ─────►  ┌──────┐  ─────►  backend A
   (curl)          │  LB  │  ─────►  backend B
           ◄─────  └──────┘  ◄─────  backend C
```

Every lesson fixes the flaw in the one before it.

| #  | Lesson                | Fixes what flaw?                          | Challenge step |
|----|-----------------------|-------------------------------------------|----------------|
| 0  | Socket round-trip     | (start) two programs talk over a socket   | —              |
| 1  | The server loop       | server died after ONE client              | Step 1 prep    |
| 2  | Concurrency (fork)    | served clients one-at-a-time (blocking)   | **Step 1**     |
| 3  | Speaking HTTP         | only spoke gibberish, not web traffic     | **Step 1**     |
| 4  | Forwarding / proxy    | talked to itself; forwarded nowhere       | **Step 1** ✅   |
| 5  | Round-robin           | only one backend = not "balancing"        | **Step 2**     |
| 6  | Health checks         | sent traffic to dead servers              | **Step 3**     |
| 7  | Assemble `lb`         | scattered demos, not one config'able tool | **Final**      |

## Folder layout

```
theories/
├── README.md            <- you are here (the map)
├── socket/              <- Lessons 0 & 1 (already built)
│   ├── socket.c            concept notes
│   ├── server.c            Lesson 0 server
│   ├── client.c            Lesson 0 client
│   └── lesson1_loop.c      Lesson 1 looping server
└── lessons/            <- Lessons 2–7
    ├── 00-prerequisites.md
    ├── 01-server-loop.md
    ├── 02-concurrency.md         + lesson2_fork.c
    ├── 03-http.md                + lesson3_http.c
    ├── 04-forwarding.md          + lesson4_proxy.c   + backend.c
    ├── 05-round-robin.md         + lesson5_roundrobin.c
    ├── 06-health-checks.md       + lesson6_health.c
    └── 07-assemble.md            + lb.c  (the final-ish program)
```

## How to use

Go lesson by lesson. For each one:
1. Read the `.md` note.
2. Compile its `.c` file:  `cc lessons/lessonX.c -o /tmp/lessonX`
3. Run it, poke it (with `curl` or `nc`), watch what happens.
4. Do the exercise at the bottom of the `.md`.
5. Move to the next lesson.

## Compile cheat-sheet

```sh
cc file.c -o out                 # most lessons
cc file.c -o out -lpthread       # lessons that use threads (2 alt, 6, 7)
```
