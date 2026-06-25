#include <stdio.h>

/* Test cases live in test_resp.c; this is the test runner's main().
   It runs the tests and EXITS — so ASan/leaks can report on it. */
void test_simple_string(void);
void test_arr(void);
void test_error(void);
void test_integers(void);

int main(void)
{
    test_simple_string(); /* "+hello world\r\n" */
    test_arr();
    test_error();
    test_integers(); /* ":-100\r\n" */

    return 0;
}
