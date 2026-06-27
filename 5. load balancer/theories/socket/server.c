#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    char buffer[1024] = {0};
    const char *hello = "Hello from server";

    // 1. create socket file descriptor
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("server_fd %d\n", server_fd);

    // attach socket to the port 8080
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; /* accpet connections on any local interfacee */
    address.sin_port = htons(8080);
    printf("%10d %10d %10d\n",
        address.sin_family,
        address.sin_addr.s_addr,
        address.sin_port
    );


    if(bind(server_fd, (struct sockaddr *) &address, addrlen) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 3. listen for connections
    if(listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    // printf("After listen\n");

    // 4. accept connection
    if((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
        perror("accept\n");
        exit(EXIT_FAILURE);
    }
    // printf("After accept\n");

    printf("new_socket descriptor: %5d\n", new_socket);
    
    // 5. read and send data
    read(new_socket, buffer, 1024);
    printf("Client message: %s\n", buffer);
    send(new_socket, hello, strlen(hello), 0);
    printf("Hello message send\n");

    // 6. close the sockets
    close(new_socket);
    close(server_fd);

    return 0;
}