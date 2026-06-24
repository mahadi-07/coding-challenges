#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "calculator.h"

#define GREEN "\033[0;32m"
#define RED   "\033[0;31m"
#define RESET "\033[0m"

int failures = 0;

#include <math.h>

#define TEST(actual, expected) \
    do { \
        double a = (actual); \
        double e = (expected); \
        if (fabs(a - e) < 1e-9) { \
            printf(GREEN "[PASS] %s\n" RESET, #actual); \
        } else { \
            printf(RED "[FAIL] %s\n" RESET, #actual); \
            printf(RED "       expected: %.10f\n" RESET, e); \
            printf(RED "       actual:   %.10f\n" RESET, a); \
            failures++; \
            exit(1); \
        } \
    } while (0)

void test_add()
{
    TEST(eval("1 + 2"), 3);
    TEST(eval("123 + 2 * 2.5"), 128);
    TEST(eval("123+ 2"), 125);
    TEST(eval("123 +2"), 125);
}

void test_sub()
{
    TEST(eval("1 - 2"), -1);
}

void test_mul()
{
    TEST(eval("1 * 2 + 5 - 10"), -3);
}

int main()
{
    test_add();
    test_sub();
    test_mul();

    if (failures == 0)
        printf(GREEN "\nAll tests passed\n" RESET);
    else
        printf(RED "\n%d test(s) failed\n" RESET, failures);
}