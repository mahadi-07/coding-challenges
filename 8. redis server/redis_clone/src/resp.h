#ifndef RESP_H
#define RESP_H

#define BASE_10 10

enum resp_type {
    SIMPLE_STRING,
    ERRORS,
    INTEGERS,
    BULK_STRINGS,
    ARRAYS
};

typedef struct RespValue RespValue;

struct RespValue {
    enum resp_type type;

    union {
        char *string;
        long i_val;

        struct {
            int count;
            RespValue **items;
        } array;
    } data;
};


RespValue *parse(const char *buf);

void free_resp(RespValue *resp);

#endif