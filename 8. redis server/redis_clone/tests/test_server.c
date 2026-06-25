#include "../src/server.h"

/* The server's entry point. Blocks forever in start_server()'s accept loop. */
int main(void)
{
    start_server(6379); /* Redis default port */
    return 0;
}
