#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"

int start_server(int port)
{
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    const char *hello = "Hello from looping server";

    // 1. create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // NEW: allow immediate reuse of the port after restart (skip TIME_WAIT)
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

    // 4. THE LOOP: serve clients forever
    while (1) {
        int conn = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (conn < 0) {
            perror("accept");
            continue;          // don't crash on one bad accept; keep serving
        }

        // who connected? convert the client's IP back to a readable string
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, sizeof(client_ip));
        printf("Got a connection from %s:%d\n", client_ip, ntohs(address.sin_port));

        char buffer[1024] = {0};
        read(conn, buffer, sizeof(buffer) - 1);
        printf("  Client said: %s\n", buffer);

        send(conn, hello, strlen(hello), 0);

        close(conn);           // close THIS client's socket, NOT server_fd
        // loop back up to accept() the next client
    }

    close(server_fd);          // unreachable here, but correct in spirit
    return 0;
}