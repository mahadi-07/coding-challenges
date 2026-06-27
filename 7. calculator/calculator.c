#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "calculator.h"

#define SIZE 100

const char *brackets = "(){}[]";
const char *opening_brackets = "({[";
const char *closing_brackets = ")}]";

struct stack_node
{
    void *item;
    struct stack_node* next;
};
struct stack_node *top = NULL;

void push(void *item)
{
    struct stack_node *nwtop = malloc(sizeof(struct stack_node));
    if(nwtop == NULL) {
        perror("malloc");
        exit(1);
    }
    nwtop->item = item;
    nwtop->next = top;
    top = nwtop;
}

void *pop()
{
    if(top == NULL) {
        fprintf(stderr, "stack underflow\n");
        exit(1);
    }

    struct stack_node *temp = top;
    top = top->next;
    
    void *item = temp->item;
    free(temp);

    return item;
}

int prec(char op)
{
    switch (op) {
        case '+':
        case '-':
            return 1;

        case '*':
        case '/':
            return 2;

        default:
            return 0;
    }
}

int isRightAssociative(char incoming_op)
{
    if(incoming_op == '^') return 1;
    else return 0;
}

int should_pop(char stack_op, char incoming_op)
{
    int p1 = prec(stack_op);
    int p2 = prec(incoming_op);

    if(p1 > p2) return 1;
    else if(p1 == p2 && !isRightAssociative(incoming_op)) return 1;
    else return 0;
}

void append_separator(char *pstfix, int *idx)
{
    if(*idx != 0) pstfix[(*idx)++] = ' ';
}

char *to_postfix(const char *expr)
{
    char *pstfix = malloc(strlen(expr) + 1);
    int idx = 0;

    const char *p = expr;
    while(*p) {
        if(*p == ' ') p++;
        else if(isdigit(*p) || *p == '.') {
            append_separator(pstfix, &idx);
            while(isdigit(*p) || *p == '.') {
                pstfix[idx++] = *p;
                p++;
            }
        }
        else if(strchr(brackets, *p) != NULL) {
            if(strchr(opening_brackets, *p) != NULL) push((void *)p);
            else if(strchr(closing_brackets, *p) != NULL) {
                while(top != NULL) {
                    if(strchr(opening_brackets, *(char *) top->item) != NULL) {
                        pop();
                        break;
                    }
                    append_separator(pstfix, &idx);
                    pstfix[idx++] = *(char *) pop();
                }
            }
            p++;
        }
        else if(*p == '+' || *p == '-' || *p == '*' || *p == '/') {
            if(top == NULL) {
                push((void *)p);
                p++;
            }
            else {
                while(top != NULL && strchr(brackets, *(char *) top->item) == NULL && should_pop(*(char *)top->item, *p)) {
                    append_separator(pstfix, &idx);
                    pstfix[idx++] = *(char *) pop();
                }
                push((void *) p);
                p++;
            }
        }
        else p++;
    }
    while(top != NULL) {
        append_separator(pstfix, &idx);
        pstfix[idx++] = *(char *) pop();
    }
    return pstfix;
}


double eval_postfix(const char *expr)
{
    top = NULL; // reset stack top

    int sz = strlen(expr);
    const char *p = expr;
    while(*p) {
        if(isspace(*p)) p++;
        else if(isdigit(*p) || *p == '.') {
            char *end;
            double d_val = strtod(p, &end);
            p = end;

            double *value = malloc(sizeof(double));
            *value = d_val;
            push((void *) value);
        }
        else {
            double op2 = *(double *) pop();
            double op1 = *(double *) pop();
            double *res = malloc(sizeof(double));

            if(*p == '+') {
                *res = op1 + op2;
                push((void *) res);
            }
            else if(*p == '*') {
                *res = op1 * op2;
                push((void *) res);
            }
            else if(*p == '-') {
                *res = op1 - op2;
                push((void *) res);
            }
            else if(*p == '/') {
                *res = op1 / op2;
                push((void *) res);
            }

            p++;
        }
    }
    return *(double *) pop();
}


double eval(const char *expr)
{
    char *pstfix = to_postfix(expr);
    double result = eval_postfix(pstfix);
    return result;
}