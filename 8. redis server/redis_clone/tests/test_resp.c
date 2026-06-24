#include <stdio.h>
#include "../src/resp.h"

void test_simple_string()
{
    const char *msg =
        "*3\r\n"
        "$3\r\nSET\r\n"
        "$4\r\nname\r\n"
        "$5\r\nhasan\r\n";

    RespValue *resp = parse(msg);

    free_resp(resp);
    
}

void test_arr()
{
    const char *msg =
        "*3\r\n"
        "$3\r\nSET\r\n"
        "$4\r\nname\r\n"
        "$5\r\nhasan\r\n";

    RespValue *arr = parse(msg);
    
    printf("Count: %5d\n", arr->data.array.count);
    for(int i = 0; i < arr->data.array.count; i++)
        printf("%12s\n", arr->data.array.items[i]->data.string);

    free_resp(arr);
}

int main()
{
    test_simple_string();
    test_arr();

    return 0;
}



// gcc src/resp.c tests/test_resp.c -o a && ./a && rm a