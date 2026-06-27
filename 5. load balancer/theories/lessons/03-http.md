# Lesson 3 — Speaking HTTP

**Code:** `lessons/lesson3_http.c`

## The flaw it fixes
So far we send custom strings. Browsers and `curl` speak **HTTP**. To be a
load balancer for web traffic, we must understand the HTTP wire format.

## What an HTTP request looks like (what curl SENDS)
Plain text. Lines end with `\r\n`. A blank line ends the headers.
```
GET /path HTTP/1.1\r\n
Host: localhost:8080\r\n
User-Agent: curl/8.0\r\n
Accept: */*\r\n
\r\n
```
- **Request line:** METHOD  PATH  VERSION
- **Headers:** `Key: Value`, one per line
- **Blank line** (`\r\n`) → headers done; body (if any) follows.

## What an HTTP response looks like (what we must SEND back)
```
HTTP/1.1 200 OK\r\n
Content-Type: text/plain\r\n
Content-Length: 13\r\n
\r\n
Hello, World!
```
- **Status line:** VERSION  CODE  REASON
- **Headers** — `Content-Length` MUST equal the body's byte count, or the
  client hangs waiting for more bytes.
- **Blank line**, then the **body**.

## Minimum viable HTTP server
1. `read()` the request into a buffer.
2. (Optionally) parse the first line to log METHOD + PATH.
3. Build a response string with a correct `Content-Length`.
4. `send()` it. `close()`.

```c
const char *body = "Hello, World!";
char resp[512];
int n = snprintf(resp, sizeof(resp),
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: %zu\r\n"
    "Connection: close\r\n"
    "\r\n"
    "%s", strlen(body), body);
send(conn, resp, n, 0);
```

`Connection: close` tells the client "I'll close after this response" —
simplest behavior; we don't have to support keep-alive yet.

## Run it & test like the challenge will
```sh
cc lessons/lesson3_http.c -o /tmp/http
/tmp/http &
curl -v http://localhost:8080/        # see request + response
curl http://localhost:8080/hello      # path is logged by the server
# or open http://localhost:8080/ in a browser
kill %1
```

## The flaw that motivates Lesson 4
This server *answers requests itself*. A load balancer doesn't answer — it
**forwards** the request to a backend and relays the backend's reply. Next
lesson: forwarding.

## Exercise
Make the server log the METHOD and PATH from the request line. Hint:
`sscanf(buffer, "%s %s", method, path);`. Hit it with `curl localhost:8080/abc`
and confirm `GET /abc` appears in the server log.
