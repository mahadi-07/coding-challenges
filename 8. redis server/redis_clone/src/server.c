#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"
#include "resp.h"
#include "db.h"
#include <pthread.h>
#include "utils.h"
#include <stdbool.h>

static void send_str(int conn, const char *s)
{
    send(conn, s, strlen(s), 0);
}

static char *resp_simple(const char *s)
{
    if (s == NULL)
        return strdup("$-1\r\n"); /* nil bulk string */

    size_t n = strlen(s);
    char *out = malloc(n + 4); /* '+' s "\r\n" '\0' */
    snprintf(out, n + 4, "+%s\r\n", s);
    return out;
}

static char *resp_integer(const int i_val)
{
    int len = snprintf(NULL, 0, ":%d\r\n", i_val);
    char *out = malloc(len + 1);
    if (out == NULL)
        return NULL;

    snprintf(out, len + 1, ":%d\r\n", i_val);
    return out;
}

static char *resp_error(const char *s)
{
    if (s == NULL)
        return strdup("$-1\r\n"); /* nil bulk string */

    size_t n = strlen(s);
    char *out = malloc(n + 4); /* '-' s "\r\n" '\0' */
    snprintf(out, n + 4, "-%s\r\n", s);
    return out;
}

static char *resp_bulk(const char *s)
{
    if (s == NULL)
        return strdup("$-1\r\n"); /* nil bulk string */
    size_t len = strlen(s);
    char *out = malloc(len + 32); /* "$<len>\r\n" s "\r\n" '\0' */
    snprintf(out, len + 32, "$%zu\r\n%s\r\n", len, s);
    return out;
}

bool is_equal_ignore_case(const char *a, const char *b)
{
    return strcasecmp(a, b) == 0;
}

char *exec_command(const char *request)
{
    RespValue *cmd = parse(request);
    char *reply = NULL;

    const char *cmd_name = NULL;
    if (cmd->type == ARRAYS && cmd->data.array.count > 0)
        cmd_name = cmd->data.array.items[0]->data.string;
    else if (cmd->type == SIMPLE_STRING)
        cmd_name = cmd->data.string;

    if (cmd_name == NULL) {
        reply = resp_error("ERR unknown command");
    } 
    else if (strcasecmp(cmd_name, "PING") == 0) {
        reply = resp_simple("PONG");
    } 
    else if (strcasecmp(cmd_name, "ECHO") == 0) {
        if (cmd->type != ARRAYS || cmd->data.array.count < 2)
            reply = resp_error("ERR wrong number of arguments for 'echo'");
        else
            reply = resp_bulk(cmd->data.array.items[1]->data.string);
    } 
    else if (strcasecmp(cmd_name, SET) == 0) {
        if (cmd->type != ARRAYS || cmd->data.array.count < 3) {
            reply = resp_error("ERR wrong number of arguments for 'set'");
        } else {
            char *key = cmd->data.array.items[1]->data.string;
            char *value = cmd->data.array.items[2]->data.string;

            uint64_t expires_at_ms = DEFAULT_EXPIRES_AT_MS;
            const char *err = NULL;
            int ok = 1;

            for (int i = 3; i < cmd->data.array.count; i++) {
                char *opt = cmd->data.array.items[i]->data.string;

                if (strcasecmp(opt, "EX")   != 0 &&
                    strcasecmp(opt, "PX")   != 0 &&
                    strcasecmp(opt, "EXAT") != 0 &&
                    strcasecmp(opt, "PXAT") != 0) {
                    err = "ERR syntax error";
                    ok = 0;
                    break;
                }

                /* each expiry flag is followed by exactly one integer */
                if (i+1 >= cmd->data.array.count) {
                    err = "ERR syntax error";
                    ok = 0;
                    break;
                }

                char *end;
                long long n = strtoll(cmd->data.array.items[++i]->data.string, &end, 10);
                if (*end != '\0' || n <= 0) {
                    err = "ERR value is not an integer or out of range"; ok = 0;
                    break;
                }

                if (strcasecmp(opt, "EX") == 0)
                    expires_at_ms = now_ms() + (uint64_t)n * 1000;
                else if (strcasecmp(opt, "PX") == 0)
                    expires_at_ms = now_ms() + (uint64_t)n;
                else if (strcasecmp(opt, "EXAT") == 0)
                    expires_at_ms = (uint64_t)n * 1000;
                else
                    expires_at_ms = (uint64_t)n;   /* PXAT */
            }

            if (!ok)
                reply = resp_error(err);
            else {
                db_set_ex(key, value, expires_at_ms);
                reply = resp_simple("OK");
            }
        }
    } 
    else if (strcasecmp(cmd_name, GET) == 0) {
        if (cmd->type != ARRAYS || cmd->data.array.count < 2)
            reply = resp_error("ERR wrong number of arguments for 'get'");
        else {
            char *value = db_get(cmd->data.array.items[1]->data.string);
            reply = resp_bulk(value);

            if(value != NULL)
                free(value);
        }
    } 
    else if(strcasecmp(cmd_name, EXISTS) == 0) {
        char *value = db_get(cmd->data.array.items[1]->data.string);
        if(value != NULL) {
            reply = resp_integer(1);
            free(value);
        }
        else reply = resp_integer(0);
    }
    else if(strcasecmp(cmd_name, DEL) == 0) {
        int del_success = 0, to_del = cmd->data.array.count - 1;
        for(int i = 1; i <= to_del; i++) {
            char *key = cmd->data.array.items[i]->data.string;
            del_success += (db_del(key) ? 1 : 0);
        }

        if(to_del == 1) 
            reply = del_success ? resp_simple("OK") : resp_integer(0);
        else
            reply = resp_integer(del_success);
    }
    else if(is_equal_ignore_case(cmd_name, INCR)) {
        char *key = cmd->data.array.items[1]->data.string;
        db_incr(key);
    }
    else {
        reply = resp_error("ERR unknown command");
    }

    free_resp(cmd);
    return reply;
}

void *per_client(void *arg)
{
    int conn = (int)(intptr_t)arg;
    char buffer[1024];

    while (1) {
        ssize_t n = read(conn, buffer, sizeof(buffer) - 1);
        if (n <= 0) break;
        buffer[n] = '\0';

        printf("User input message : %s\n\n\n", buffer);

        char *reply = exec_command(buffer);   /* parse + dispatch + format */
        send_str(conn, reply);
        free(reply);
    }
    close(conn);
    return NULL;
}

static void handle_client(int conn)
{
    pthread_t tid;
    if (pthread_create(&tid, NULL, per_client, (void *)(intptr_t)conn) != 0) {
        perror("pthread_create");
        close(conn);
    }
    pthread_detach(tid);
}

int start_server(int port)
{
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // 1. create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // allow immediate reuse of the port after restart (skip TIME_WAIT)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 2. describe + bind the address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 3. listen
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d (Ctrl+C to stop)...\n", port);

    // 4. serve clients forever
    while (1) {
        int conn = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (conn < 0) {
            perror("accept");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, sizeof(client_ip));
        printf("Got a connection from %s:%d\n", client_ip, ntohs(address.sin_port));

        handle_client(conn); // read / parse / reply until the client leaves
    }

    close(server_fd); // unreachable — the server runs until killed
    return 0;
}
