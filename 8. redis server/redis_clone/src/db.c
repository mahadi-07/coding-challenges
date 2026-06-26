#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Entry Entry;
struct Entry
{
    char *key;
    char *value;
    struct Entry *next;
};

#define HASH_SIZE 1000
static Entry *hashtab[HASH_SIZE];

unsigned hash(const char *s) {
    unsigned long h = 5381;
    
    int c;
    while ((c = (unsigned char)*s++))
        h = ((h << 5) + h) + c;
    
    return h % HASH_SIZE;
}

Entry *lookup(const char *key)
{
    Entry *p;
    for(p = hashtab[hash(key)]; p != NULL; p = p->next)
        if(strcmp(key, p->key) == 0)
            return p;

    return p;
}

Entry *install(const char *key, const char *value)
{
    Entry *p;
    if((p = lookup(key)) == NULL) {
        p = malloc(sizeof(Entry));
        
        if((p->key = strdup(key)) == NULL)
            return NULL;
        
        unsigned long hashval = hash(p->key);
        p->next = hashtab[hashval];
        hashtab[hashval] = p;
    }
    else
        free(p->value);
    
    if((p->value = strdup(value)) == NULL)
        return NULL;

    return p;
}

void undef(const char *key)
{
    Entry *cur;
    Entry *prev = NULL;
    for(cur = hashtab[hash(key)]; cur != NULL; cur = cur->next) {
        if(strcmp(cur->key, key) == 0) {
            if(prev == NULL) /* Need to remove first entry */
                hashtab[hash(cur->key)] = cur->next;
            else
                prev->next = cur->next;

            free(cur->key);
            free(cur->value);
            free(cur);
            break;
        }
        prev = cur;
    }
}

// void db_set(const char *key, const char *value)
// {

// }






void db_init() {

}