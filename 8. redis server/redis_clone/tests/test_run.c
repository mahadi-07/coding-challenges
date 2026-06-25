#include <stdio.h>

/* Test cases live in test_resp.c (parser) and test_server.c (commands).
   This is the test runner's main() — it runs all tests and EXITS, so ASan/leaks
   can report on it. */
void test_simple_string(void);
void test_arr(void);
void test_error(void);
void test_integers(void);

void test_ping(void);
void test_echo(void);
void test_echo_empty(void);
void test_unknown_command(void);
void test_command_case_insensitive(void);

int main(void)
{
    /* RESP parser tests */
    test_simple_string(); /* "+hello world\r\n" */
    test_arr();
    test_error();
    test_integers(); /* ":-100\r\n" */

    /* Server command tests */
    test_ping();
    test_echo();
    test_echo_empty();
    test_unknown_command();
    test_command_case_insensitive();

    return 0;
}
