#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../src/resp.h"

#define GREEN "\033[32m"
#define BOLD  "\033[1m"
#define RESET "\033[0m"

#define TEST_PASSED(name) \
    printf(GREEN "[PASS]" RESET " " BOLD "%s" RESET "\n", (name))

void test_simple_string()
{
    const char *msg = "+hello world\r\n";

    RespValue *simple = parse(msg);

    assert(simple->type == SIMPLE_STRING);
    assert(strcmp(simple->data.string, "hello world") == 0);
    free_resp(simple);
    
    TEST_PASSED("test_simple_string");
}

void test_arr()
{
    /* Case-1 */
    char *msg =
        "*3\r\n"
        "$3\r\nSET\r\n"
        "$4\r\nname\r\n"
        "$5\r\nhasan\r\n";

    RespValue *arr = parse(msg);
    assert(arr->data.array.count == 3);
    assert(strcmp(arr->data.array.items[0]->data.string, "SET") == 0);
    assert(strcmp(arr->data.array.items[1]->data.string, "name") == 0);
    assert(strcmp(arr->data.array.items[2]->data.string, "hasan") == 0);
    free_resp(arr);

    /* Case-2 */
    msg = "*2\r\n$4\r\necho\r\n$11\r\nhello world\r\n";

    arr = parse(msg);
    assert(arr->data.array.count == 2);
    assert(strcmp(arr->data.array.items[0]->data.string, "echo") == 0);
    assert(strcmp(arr->data.array.items[1]->data.string, "hello world") == 0);
    free_resp(arr);

    /* Case-3 */
    msg = "*2\r\n$3\r\nget\r\n$3\r\nkey\r\n";

    arr = parse(msg);
    assert(arr->data.array.count == 2);
    assert(strcmp(arr->data.array.items[0]->data.string, "get") == 0);
    assert(strcmp(arr->data.array.items[1]->data.string, "key") == 0);
    free_resp(arr);

    /* Case-4 */
    msg = "*-1\r\n";

    arr = parse(msg);
    assert(arr->type == ARRAYS);
    assert(arr->data.array.count == -1);
    free_resp(arr);

    TEST_PASSED("test_arr");

}

void test_error()
{
    char *msg = "-Error message\r\n";
    RespValue *err = parse(msg);

    assert(err->type == ERRORS);
    assert(strcmp(err->data.string, "Error message") == 0);
    free_resp(err);

    TEST_PASSED("test_error");
}

void test_integers()
{
    char *msg = ":-100\r\n";

    RespValue *arr = parse(msg);
    assert(arr->type == INTEGERS);
    assert(arr->data.i_val == -100);
    free_resp(arr);

    TEST_PASSED("test_integers");
}

// gcc src/resp.c tests/test_resp.c -o a && ./a && rm a