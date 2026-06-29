#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

enum http_method {
    GET,
    POST
};

int main()
{
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("sockeet failed");
        exit(EXIT_FAILURE);
    }

    // allow immediate reuse of the port after restart (skip TIME_WAIT)
    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(80);;

    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d (Ctrl+C to stop)...\n", 80);

    while(1) {
        int conn = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if(conn < 0) {
            perror("accept");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, sizeof(client_ip));
        printf("Got a connection from %s:%d\n", client_ip, ntohs(address.sin_port));

        FILE *fp = fdopen(dup(conn), "r");
        if(fp == NULL) {
            perror("fdopen");
            close(conn);
            continue;
        }

        // enum http_method method_type = GET;
        int first_line = 1;
        
        char method[16];
        char path[256];
        char version[16];

        char line[1024] = {0};
        while((fgets(line, sizeof(line), fp)) != NULL) {
            if (strcmp(line, "\r\n") == 0)
                break;

            if (first_line) {
                if (sscanf(line, "%15s %255s %15s", method, path, version) != 3) {
                    fprintf(stderr, "Invalid request line\n");
                    fclose(fp);
                    close(conn);
                    exit(1);
                }
                first_line = 0;
            }
        }
        fclose(fp);

        printf("%s %d\n", path, strlen(path));

        char body[1024];
        snprintf(body, sizeof(body), "Requested path: %s", path);

        char response[2048];
        snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "%s",
            strlen(body),
            body
        );

        write(conn, response, strlen(response));

        close(conn);
    }

    return 0;
}