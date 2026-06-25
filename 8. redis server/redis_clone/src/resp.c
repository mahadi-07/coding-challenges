#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "resp.h"

enum resp_type get_identify_type(const char *buf)
{
    switch (buf[0]) {
        case '+':
            return SIMPLE_STRING;

        case '-':
            return ERRORS;

        case ':':
            return INTEGERS;

        case '$':
            return BULK_STRINGS;

        case '*':
            return ARRAYS;

        default:
            perror("invalid type");
            exit(1);
    }
}

RespValue *make_string(char *str)
{
    RespValue *v = malloc(sizeof(* v));
    if(!v)
        return NULL;

    v->type = SIMPLE_STRING;
    v->data.string = strdup(str);
    
    return v;
}

/**
 * Simple string
 */
RespValue *parse_simple_string(const char *buf)
{
    char *word = malloc(strlen(buf) + 1);
    char *p = word;
    buf++;
    while(*buf) {
        if(*buf == '\r' && *(buf+1) == '\n')
            break;
        else
            *p++ = *buf++;
    }
    *p = '\0';

    RespValue *resp = make_string(word);
    
    free(word);
    
    return resp;
}

RespValue *parse_error(const char *buf)
{
    RespValue *err = parse_simple_string(buf);
    err->type = ERRORS;
    return err;
}

RespValue *parse_integer(const char *buf)
{
    RespValue *i_resp = parse_simple_string(buf);

    long i_val = strtol(i_resp->data.string, NULL, BASE_10);
    free(i_resp->data.string);

    i_resp->type = INTEGERS;
    i_resp->data.i_val = i_val;

    return i_resp;
}

/**
 * Array
 */
char *get_word(const char *buf, int sz)
{
    char *word = malloc(sz + 1);
    if(word == NULL)
        return NULL;

    memcpy(word, buf, sz);
    word[sz] = '\0';
    return word;
}

RespValue *resp_alloc_arr(int sz, enum resp_type type)
{
    RespValue *resp = malloc(sizeof(*resp));
    if(resp == NULL)
        return NULL;

    resp->type = type;
    resp->data.array.count = sz;
    if(sz > 0) resp->data.array.items = malloc(sz * sizeof(struct RespValue *));
    return resp;
} 

RespValue *parse_array(const char *buf)
{
    RespValue *arr;
    int arr_sz = 0, i = 0;
    char *endptr;
    while(*buf) {
        if(*buf == '*') {
            arr_sz = strtol(buf+1, &endptr, BASE_10);
            buf = (endptr+2);
            arr = resp_alloc_arr(arr_sz, ARRAYS);
            if(arr_sz == -1) return arr; /* "*-1\r\n" represent Null Array */
        }
        else if(*buf == '$') {
            int str_sz = strtol(buf+1, &endptr, BASE_10);
            buf = endptr;
            char *word = get_word(buf+2, str_sz);
            buf += (2+str_sz+2);
            
            if(i >= arr_sz) {
                perror("out of index");
                exit(1);
            }
            arr->data.array.items[i++] = make_string(word);
            free(word);
        }
        else ++buf;
    }
    return arr;
}



RespValue *parse(const char *buf)
{
    // printf("\nparse: %50s\n\n", buf);
    // printf("\n type: %d\n", get_identify_type(buf));

    switch (get_identify_type(buf))
    {
        case SIMPLE_STRING:
            return parse_simple_string(buf);
        case ARRAYS:
            return parse_array(buf);
        case ERRORS:
            return parse_error(buf);
        case INTEGERS:
            return parse_integer(buf);
        
        default:
            perror("unknown type");
            exit(1);
    }
}

void free_resp(RespValue *resp)
{
    if (resp == NULL)
        return;

    switch (resp->type) {
        case SIMPLE_STRING:
        case ERRORS:
        case BULK_STRINGS:
            free(resp->data.string);
            break;

        case INTEGERS:
            /* nothing dynamically allocated */
            break;

        case ARRAYS:
            if(resp->data.array.count > 0) {
                for (int i = 0; i < resp->data.array.count; i++) {
                    free_resp(resp->data.array.items[i]);
                }
                free(resp->data.array.items);
            }
            break;
    }

    free(resp);
}