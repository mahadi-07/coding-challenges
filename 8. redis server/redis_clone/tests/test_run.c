#include <stdio.h>

/* Test cases live in test_resp.c (parser), test_server.c (commands) and
   test_db.c (hash table). This is the test runner's main() — it runs all tests
   and EXITS, so ASan/leaks can report on it. */
void test_simple_string(void);
void test_arr(void);
void test_error(void);
void test_integers(void);

void test_ping(void);
void test_echo(void);
void test_echo_empty(void);
void test_unknown_command(void);
void test_command_case_insensitive(void);
void test_set_without_ttl(void);
void test_get_without_ttl();
void test_get_with_ttl();

int run_db_tests(void);

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
    test_set_without_ttl();
    test_get_without_ttl();
    test_get_with_ttl();

    // /* DB (hash table) tests */
    int failed = run_db_tests();

    return failed > 0 ? 1 : 0;
}
