#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "db.h"

#define KEY_SZ 1024
#define VALUE_SZ 1024
#define ITEM_SZ 1024

static char *KEY_VALUE_SEPARATOR = ":";
static const char *ITEM_SEPARATOR = "\r\n";

static unsigned long ht_hash(const char *key)
{
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*key++))
        h = ((h << 5) + h) + c;
    return h % HT_BUCKETS;
}

hashtable_t *ht_new(void)
{
    hashtable_t *ht = calloc(1, sizeof(*ht));
    if(ht == NULL)
        return NULL;

    for(size_t i = 0; i < HT_BUCKETS; i++)
        ht->buckets[i] = NULL;

    return ht;
}

void ht_free(hashtable_t *ht)
{
    for(size_t i = 0; i < HT_BUCKETS; i++) {
        note_t *cur = ht->buckets[i];
        while(cur != NULL) {
            note_t *next = cur->next;

            free(cur->key);
            free(cur->payload.value);
            free(cur);

            cur = next;
        }
    }
    
    free(ht);
}

/**
 * Return the entry associated with `key`, or NULL if it does not exist.
 */
static note_t *lookup(const hashtable_t *ht, const char *key)
{
    note_t *p;
    for(p = ht->buckets[ht_hash(key)]; p != NULL; p = p->next)
        if(strcmp(key, p->key) == 0)
            return p;

    return p;
}

void ht_set(hashtable_t *ht, const char *key, const char *value)
{
    note_t *p = lookup(ht, key);

    if(p == NULL) {
        p = calloc(1, sizeof(note_t));
        if(p == NULL)
            return;

        if((p->key = strdup(key)) == NULL) {
            free(p);
            return;
        }
                
        unsigned long hval = ht_hash(key);
        p->next = ht->buckets[hval];
        ht->buckets[hval] = p;
    }

    char *new_value = strdup(value);
    if(new_value == NULL)
        return;

    free(p->payload.value);
    p->payload.value = new_value;
    
    return;
}

const char *ht_get(const hashtable_t *ht, const char *key)
{
    note_t *p = lookup(ht, key);
    if(p != NULL)
        return p->payload.value;
    
    return NULL;
}

static char *_get_path(const char *dir)
{
    if(strncasecmp(dir, "./", 2) != 0) {
        perror("invalid path");
        exit(1);
    }

    char path[1024];
    int n = snprintf(path, sizeof(path), "%s", dir);
    if(n < 0 || (size_t)n >= sizeof(path))
        exit(1);
    
    return strdup(path);
}

int db_save(const hashtable_t *ht, const char *dir)
{
    char *path = _get_path(dir);
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    free(path);
    if(fd == -1) {
        perror("error opening file");
        exit(1);
    }

    lseek(fd, 0, SEEK_END);

    for(size_t i = 0; i < HT_BUCKETS; i++) {
        note_t *cur = ht->buckets[i];

        while(cur != NULL) {
            char item[ITEM_SZ] = {};
            snprintf(item, sizeof(item),
                "%s%s%s%s",
                cur->key,
                KEY_VALUE_SEPARATOR,
                cur->payload.value,
                ITEM_SEPARATOR);

            write(fd, item, strlen(item));

            cur = cur->next;
        }

    }

    close(fd);
    return 0;
}

int db_load(hashtable_t *ht, const char *dir)
{
    char *path = _get_path(dir);
    FILE *fp = fopen(path, "r");
    free(path);
    if(fp == NULL)
        return -1;

    char item[ITEM_SZ] = {};
    while (fgets(item, sizeof(item), fp) != NULL) {
        item[strcspn(item, ITEM_SEPARATOR)] = '\0';

        char *value = strstr(item, KEY_VALUE_SEPARATOR);
        if(value == NULL)
            continue;
        *value = '\0';

        value += strlen(KEY_VALUE_SEPARATOR);
        char *key = item;

        ht_set(ht, key, value);
    }

    fclose(fp);
    return 0;
}