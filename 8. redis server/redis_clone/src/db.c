#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include "utils.h"
#include "db.h"     /* public API; also pulls in db_dto.h (Entry, DbResult) */

#define HASH_SIZE 1000
static Entry *hashtab[HASH_SIZE];

static pthread_mutex_t db_lock = PTHREAD_MUTEX_INITIALIZER;

/* djb2 string hash, folded into the table size. */
unsigned hash(const char *s) {
    unsigned long h = 5381;

    int c;
    while ((c = (unsigned char)*s++))
        h = ((h << 5) + h) + c;

    return h % HASH_SIZE;
}

/* True if e's TTL has already elapsed. expires_at_ms == 0 means "no expiry",
   so a zero TTL never reads as expired even when now_ms() is large. */
static int expired(const Entry *e)
{
    return e->expires_at_ms != 0 && e->expires_at_ms <= now_ms();
}

/* Return the entry associated with `key`, or NULL if it does not exist. */
Entry *lookup(const char *key)
{
    Entry *p;
    for(p = hashtab[hash(key)]; p != NULL; p = p->next)
        if(strcmp(key, p->key) == 0)
            return p;

    return p;
}

/*
 * Insert or update an entry.
 *
 * Replaces any existing value and expiration time. The caller must hold
 * db_lock.
 *
 * Returns the updated entry, or NULL on allocation failure.
 */
Entry *install(const char *key, const char *value, uint64_t expires_at_ms)
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

/*
 * Remove the entry associated with `key`.
 *
 * Frees all memory owned by the entry. The caller must hold db_lock.
 *
 * Returns true if an entry was removed; otherwise false.
 */
bool undef(const char *key)
{
    Entry *cur;
    Entry *prev = NULL;
    for(cur = hashtab[hash(key)]; cur != NULL; cur = cur->next) {
        if(strcmp(cur->key, key) == 0) {
            if(prev == NULL)
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

char *db_get(const char *key)
{
    pthread_mutex_lock(&db_lock);
    Entry *p = lookup(key);

    char *copy;

    if(p == NULL)
        copy = NULL;
    else if(expired(p)) {
        undef(p->key);   /* lazy delete on read */
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

DbResult *db_incr(const char *key)
{
    pthread_mutex_lock(&db_lock);
    DbResult *res = malloc(sizeof(DbResult));
    res->value = NULL;   /* NULL until success, so the caller can free() unconditionally */
    res->error = NULL;

    Entry *p = get(key);
    if(p == NULL)
        p = install(key, "1", 0);   /* missing key: Redis seeds 0 then +1 -> 1 */
    else {
        if(!is_number(p->value)) {
            pthread_mutex_unlock(&db_lock);

            res->ok = false;
            res->error = "value is not an integer or out of range";
            return res;
        }

        char new_value[63];
        snprintf(new_value, sizeof(new_value), "%d", atoi(p->value) + 1);
        p = install(key, new_value, p->expires_at_ms);
    }

    if(p == NULL) {
        pthread_mutex_unlock(&db_lock);

        res->ok = false;
        res->error = "failed to save";
        return res;
    }

    res->ok = true;
    res->value = strdup(p->value);
    pthread_mutex_unlock(&db_lock);

    return res;
}
