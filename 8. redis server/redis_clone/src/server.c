#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"
#include "resp.h"
#include "db.h"
#include <pthread.h>
#include "utils.h"

static void send_str(int conn, const char *s)
{
    send(conn, s, strlen(s), 0);
}

static void send_resp_simple_string(int conn, const char *value)
{
    if (value != NULL) {
        send_str(conn, "+");
        send_str(conn, value);
        send_str(conn, "\r\n");
    } else
        send_str(conn, "$-1\r\n");
}

void *per_client(void *arg)
{
    int conn = (int)(intptr_t)arg;
    char buffer[1024];

    while(1) {
        ssize_t n = read(conn, buffer, sizeof(buffer) - 1);
        if(n <= 0) break;
        buffer[n] = '\0';

        RespValue *cmd = parse(buffer);

        const char *cmd_name = NULL;
        if(cmd->type == ARRAYS) cmd_name = cmd->data.array.items[0]->data.string;
        else if(cmd->type == SIMPLE_STRING) cmd_name = cmd->data.string;
        
        printf("command name: %s\n", cmd_name);

        if (strcasecmp(cmd_name, "PING") == 0)
            send_str(conn, "+PONG\r\n");
        else if (strcasecmp(cmd_name, "ECHO") == 0) {
            if (cmd->data.array.count < 2)
                send_str(conn, "-ERR wrong number of arguments for 'echo'\r\n");
            else {
                char *arg = cmd->data.array.items[1]->data.string;
                char reply[2048];
                int len = snprintf(reply, sizeof(reply), "$%zu\r\n%s\r\n", strlen(arg), arg);
                send(conn, reply, len, 0);
            }
        }
        else if(strcasecmp(cmd_name, SET) == 0) {
            if(cmd->data.array.count < 3) {
                send_str(conn, "-ERR wrong number of arguments for 'set'\r\n");
            }
            else {
                char *key = cmd->data.array.items[1]->data.string;
                char *value = cmd->data.array.items[2]->data.string;

                uint64_t expires_at_ms = DEFAULT_EXPIRES_AT_MS;
                const char *err = NULL;
                int ok = 1;
                
                for(int i = 3; i < cmd->data.array.count; i++) {
                    char *opt = cmd->data.array.items[i]->data.string;
                    
                    if(strcasecmp(opt, "EX")   != 0 &&
                        strcasecmp(opt, "PX")   != 0 &&
                        strcasecmp(opt, "EXAT") != 0 &&
                        strcasecmp(opt, "PXAT") != 0) {
                        err = "-ERR syntax error\r\n"; ok = 0; break;
                    }

                    /* each expiry flag is followed by exactly one integer */
                    if(i + 1 >= cmd->data.array.count) {
                        err = "-ERR syntax error\r\n"; ok = 0; break;
                    }

                    char *end;
                    long long n = strtoll(cmd->data.array.items[++i]->data.string, &end, 10);
                    if(*end != '\0' || n <= 0) {
                        err = "-ERR value is not an integer or out of range\r\n"; ok = 0;
                        break;
                    }

                    if(strcasecmp(opt, "EX") == 0)
                        expires_at_ms = now_ms() + (uint64_t)n * 1000;
                    else if(strcasecmp(opt, "PX")   == 0)
                        expires_at_ms = now_ms() + (uint64_t)n;
                    else if(strcasecmp(opt, "EXAT") == 0)
                        expires_at_ms = (uint64_t)n * 1000;
                    else
                        expires_at_ms = (uint64_t)n;
                }

                if(!ok)
                    send_str(conn, err);
                else {
                    db_set_ex(key, value, expires_at_ms);
                    send_str(conn, "+OK\r\n");
                }
            }
        }
        else if(strcasecmp(cmd_name, GET) == 0) {
            char *key = cmd->data.array.items[1]->data.string;
            char *value  = db_get(key);
            send_resp_simple_string(conn, value);
            free(value);
        }
        else
            send_resp_simple_string(conn, "-ERR unknown command");

        free_resp(cmd);
    }
    close(conn);
    return NULL;
}

static void handle_client(int conn)
{
    // printf("\ncreating pthread_t\n");
    pthread_t tid;
    if(pthread_create(&tid, NULL, per_client, (void *) (intptr_t) conn) != 0) {
        perror("pthread_create");
        close(conn);
    }
    pthread_detach(tid);
    // printf("\npthread_detach\n");
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

        handle_client(conn);   // read / parse / reply until the client leaves
        printf("\n\nClient accpeted async\n\n");
    }

    close(server_fd);   // unreachable — the server runs until killed
    return 0;
}
