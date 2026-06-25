#include "server.h"

/* Server entry point: listen on :6379 and serve forever (blocks). */
int main(void)
{
    start_server(6379);
    return 0;
}
