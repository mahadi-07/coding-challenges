#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef struct {
    char host[64];
    int port;
} Backend;

Backend backends[] = {
    {"127.0.0.1", 8091},
    {"127.0.0.1", 8092},
    {"127.0.0.1", 8093}
};

int backend_count = sizeof(backends) / sizeof(Backend);

int next_backend(void)
{
    static int i = 0;
    
    int chosen = i++;
    i %= backend_count;
    return chosen;
}

int connect_backend(const char *host, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port)
    };

    inet_pton(AF_INET, host, &addr.sin_addr);

    if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd,
                SOL_SOCKET,
                SO_REUSEADDR,
                &opt,
                sizeof(opt));
    
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(8080),
        .sin_addr.s_addr = INADDR_ANY
    };

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 128);
    
    printf("LB listening on 8080\n");

    while(1) {
        int client = accept(server_fd, NULL, NULL);
        int idx = next_backend();
        Backend *b = &backends[idx];

        printf("Forwarding -> %s:%d\n",
                b->host,
                b->port);

        int backend = connect_backend(b->host, b->port);

        if(backend < 0) {
            char *err = 
                "HTTP/1.1 502 Bad Gateway\r\n"
                "Content-Length: 0\r\n\r\n";
            
            write(client, err, sizeof(err));
            close(client);
            continue;
        }

        char buffer[8192];

        ssize_t n = read(client, buffer, sizeof(buffer));
        write(backend, buffer, n);

        while((n = read(backend, buffer, sizeof(buffer))) > 0) {
            write(client, buffer, n);
        }

        close(backend);
        close(client);
    }
}