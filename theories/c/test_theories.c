#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum resp_type {
    SIMPLE_STRING,
    ERRORS,
    INTEGERS,
    BULK_STRINGS,
    ARRAYS
};

typedef struct RespValue Rv;
struct RespValue {
    enum resp_type type;

    union {
        char *string;
        long integer;

        struct {
            int count;
            Rv **items;
        } array;
    } data;
};

Rv *arr_alloc()
{
    Rv *v = malloc(sizeof(Rv));

    v->type = ARRAYS;

    Rv *item1 = malloc(sizeof(Rv));
    printf("( item1 ) head address: %50p\n", item1);
    printf("( item1 ) address stack address (local): %33p\n", &item1);

    item1->type = SIMPLE_STRING;

    char msg[] = "item no. 1";
    char *msg = "item no. 1";
    
    item1->data.string = msg;

    v->data.array.count = 1;
  
    v->data.array.items = malloc(sizeof(Rv *));
    v->data.array.items[0] = item1;

    return v;
}

int main()
{
    Rv *v = arr_alloc();
    printf("\n\n\n");

    printf("count: %d\n", v->data.array.count);
    for(int i = 0; i < v->data.array.count; i++) {
        printf("message: %10s\n", v->data.array.items[i]->data.string);
    }

    return 0;
}