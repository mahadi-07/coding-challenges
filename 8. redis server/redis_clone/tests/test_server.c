#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

void test_set_without_ttl()
{
    char *msg = "*3\r\n$3\r\nSET\r\n$4\r\nname\r\n$5\r\nhasan\r\n";
    char *r = exec_command(msg);
    assert(strcmp(r, "+OK\r\n") == 0);
    free(r);
    TEST_PASSED("server: test_set_without_ttl");
}

void test_get_without_ttl()
{
    char *msg = "*3\r\n$3\r\nSET\r\n$4\r\nname\r\n$5\r\nhasan\r\n";
    char *r = exec_command(msg);
    assert(strcmp(r, "+OK\r\n") == 0);

    msg = "*2\r\n$3\r\nget\r\n$4\r\nname\r\n";
    r = exec_command(msg);
    assert(strcmp(r, "$5\r\nhasan\r\n") == 0);

    free(r);
    TEST_PASSED("server: test_get_without_ttl");
}

void test_get_with_ttl()
{
    /* SET name=hasan with a 2-second TTL */
      char *msg = "*5\r\n$3\r\nSET\r\n$4\r\nname\r\n$5\r\nhasan\r\n$2\r\nEX\r\n$1\r\n2\r\n";
      char *r = exec_command(msg);
      assert(strcmp(r, "+OK\r\n") == 0);
      free(r);
      
      /* within the TTL, the value is still there */
      r = exec_command("*2\r\n$3\r\nget\r\n$4\r\nname\r\n");
      assert(strcmp(r, "$5\r\nhasan\r\n") == 0);
      free(r);

      /* once the TTL elapses, the key is gone -> nil bulk string */
      sleep(3);
      r = exec_command("*2\r\n$3\r\nget\r\n$4\r\nname\r\n");
      assert(strcmp(r, "$-1\r\n") == 0);
      free(r);

      TEST_PASSED("server: test_get_with_ttl");
}

// redis-benchmark -t ping -q -n 1000 -c 10