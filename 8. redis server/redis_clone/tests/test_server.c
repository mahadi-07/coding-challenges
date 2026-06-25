#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/server.h"

#define GREEN "\033[32m"
#define BOLD  "\033[1m"
#define RESET "\033[0m"
#define TEST_PASSED(name) printf(GREEN "[PASS]" RESET " " BOLD "%s" RESET "\n", (name))

/* Server command tests. Each feeds a raw RESP request to exec_command() and
   checks the reply. exec_command() is pure logic (no sockets), so these are
   fast and deterministic. The networking is verified separately with nc/redis-cli. */

void test_ping(void)
{
    char *r = exec_command("*1\r\n$4\r\nPING\r\n");
    assert(strcmp(r, "+PONG\r\n") == 0);
    free(r);
    TEST_PASSED("server: test_ping");
}

void test_echo(void)
{
    char *r = exec_command("*2\r\n$4\r\nECHO\r\n$11\r\nHello World\r\n");
    assert(strcmp(r, "$11\r\nHello World\r\n") == 0);
    free(r);
    TEST_PASSED("server: test_echo");
}

void test_echo_empty(void)
{
    /* ECHO of an empty bulk string replies with $0\r\n\r\n */
    char *r = exec_command("*2\r\n$4\r\nECHO\r\n$0\r\n\r\n");
    assert(strcmp(r, "$0\r\n\r\n") == 0);
    free(r);
    TEST_PASSED("server: test_echo_empty");
}

void test_unknown_command(void)
{
    char *r = exec_command("*1\r\n$3\r\nFOO\r\n");
    assert(strcmp(r, "-ERR unknown command\r\n") == 0);
    free(r);
    TEST_PASSED("server: test_unknown_command");
}

void test_command_case_insensitive(void)
{
    char *r = exec_command("*1\r\n$4\r\nping\r\n");  /* lowercase */
    assert(strcmp(r, "+PONG\r\n") == 0);
    free(r);
    TEST_PASSED("server: test_command_case_insensitive");
}
