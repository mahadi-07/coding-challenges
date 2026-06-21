#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <port> <name>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    char *name = argv[2];

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);

    printf("%s listening on %d\n", name, port);

    while(1) {
        struct sockaddr_in cli;
        socklen_t clilen = sizeof(cli);
        int conn = accept(server_fd, (struct sockaddr *)&cli, &clilen);

        char buf[4096];
        read(conn, buf, sizeof(buf));

        char response[512];
        snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "%s\n",
            strlen(name)+1,
            name);

        write(conn, response, strlen(response));
        close(conn);
    }
    exit(0);
}