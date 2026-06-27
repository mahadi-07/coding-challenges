#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include "utils.h"

typedef struct Entry Entry;
struct Entry
{
    char *key;
    char *value;
    u_int64_t expires_at_ms;
    struct Entry *next;
};

#define HASH_SIZE 1000
static Entry *hashtab[HASH_SIZE];

static pthread_mutex_t db_lock = PTHREAD_MUTEX_INITIALIZER;

unsigned hash(const char *s) {
    unsigned long h = 5381;
    
    int c;
    while ((c = (unsigned char)*s++))
        h = ((h << 5) + h) + c;
    
    return h % HASH_SIZE;
}

static int expired(const Entry *e)
{
    return e->expires_at_ms != 0 && e->expires_at_ms <= now_ms();
}

Entry *lookup(const char *key)
{
    Entry *p;
    for(p = hashtab[hash(key)]; p != NULL; p = p->next)
        if(strcmp(key, p->key) == 0)
            return p;

    return p;
}

Entry *install(const char *key, const char *value, u_int64_t expires_at_ms)
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

    p->expires_at_ms = expires_at_ms; /* 0 == no expiry; also clears any old TTL */
    return p;
}

bool undef(const char *key)
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
            return true;
        }
        prev = cur;
    }
    return false;
}

void db_set(const char *key, const char *value)
{
    pthread_mutex_lock(&db_lock);
    Entry *p = install(key, value, 0);
    pthread_mutex_unlock(&db_lock);

    if(p == NULL)
        perror("unable to set value");
}

void db_set_ex(const char *key, const char *value, uint64_t expires_at_ms)
{
   pthread_mutex_lock(&db_lock);
   Entry *p = install(key, value, expires_at_ms);
   pthread_mutex_unlock(&db_lock);

   if(p == NULL)
        perror("unable to set value");
}

/**
 * @brief Retrieves the value associated with the specified key.
 *
 * Looks up the given key in the database. If the key exists and has not
 * expired, a newly allocated copy of its value is returned. If the key does
 * not exist or has expired, NULL is returned.
 *
 * @param key The key to retrieve.
 *
 * @return A newly allocated copy of the value associated with the key, or
 *         NULL if the key does not exist or has expired.
 *
 * @note The returned string is allocated with `strdup()`. The caller owns
 *       the returned memory and is responsible for freeing it with `free()`
 *       when it is no longer needed.
 */
char *db_get(const char *key)
{
    pthread_mutex_lock(&db_lock);
    Entry *p = lookup(key);
    
    char *copy;

    if(p == NULL)
        copy = NULL;
    else if(expired(p)) {
        undef(p->key);
        copy = NULL;
    }
    else
        copy = strdup(p->value);

    pthread_mutex_unlock(&db_lock);
    return copy;
}

static Entry *get(const char *key)
{
    Entry *p = lookup(key);
    return p;
}

bool db_del(const char *key)
{
    return undef(key);
}

int db_incr(const char *key)
{
    pthread_mutex_lock(&db_lock);
    Entry *p = get(key);
    if(p == NULL)
        p = install(key, "0", 0);
    else {
        char new_value[63];
        snprintf(new_value, sizeof(new_value), "%d", atoi(p->value) + 1);
        p = install(key, new_value, p->expires_at_ms);
    }

    if(p == NULL) {
        perror("failed to save");
        pthread_mutex_unlock(&db_lock);
        return -1;
    }

    int i_val = atoi(p->value);
    pthread_mutex_unlock(&db_lock);

    return i_val;
}