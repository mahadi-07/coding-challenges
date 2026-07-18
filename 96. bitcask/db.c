#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "helper.h"
#include "db.h"
#include <time.h>
#include <stdint.h>
#include <sys/types.h>

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
        return NULL;
    }

    char path[1024];
    int n = snprintf(path, sizeof(path), "%s", dir);
    if(n < 0 || (size_t)n >= sizeof(path))
        return NULL;
    
    return strdup(path);
}

char *_get_cask_path(const char *dir, const int cask_num)
{
    char *path = _get_path(dir);
    if(path == NULL)
        return NULL;
        
    char cask_path[1024];
    snprintf(cask_path, sizeof(cask_path), "%s/cask.%d", path, cask_num);

    free(path);
    return strdup(cask_path);
}

int db_save(const hashtable_t *ht, const char *dir)
{
    char *path = _get_path(dir);
    if(path == NULL)
        return 0;

    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    free(path);
    if(fd == -1) {
        perror("error opening file");
        return 0;
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
    if(path == NULL)
        return 0;
        
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

bitcask_t *bitcask_open(const char *dir, const size_t threshold)
{
    char *path = NULL;
    int active_file_id = 0;

    for(int i = 100; i >= 0; i--) {
        path = _get_cask_path(dir, i);
        if(path == NULL)
            continue;

        if(access(path, F_OK) == 0) {
            active_file_id = i;
            free(path);
            break;
        }

        free(path);
    }

    path = _get_cask_path(dir, active_file_id);
    if(path == NULL)
        return NULL;

    int fd = open(path, O_RDWR | O_CREAT | O_APPEND, 0644);
    free(path);

    if(fd == -1) {
        perror("Error opening file");
        return NULL;
    }
        
    bitcask_t *cask = calloc(1, sizeof(*cask));
    if(cask == NULL)
        return NULL;

    cask->dir = strdup(dir);
    cask->threshold = threshold;
    cask->active_fd = fd;
    cask->active_file_id = active_file_id;

    return cask;
}

void bitcask_close(bitcask_t *cask)
{
    if(cask == NULL)
        return;

    close(cask->active_fd);
    free(cask->dir);
}

long bitcask_append(bitcask_t *db, const char *key, const char *value)
{

	size_t key_len = strlen(key);
	size_t value_len = strlen(value);
	size_t body_without_crc = 16 + key_len + value_len;
	size_t total = 4 + body_without_crc;

	unsigned char *buf = malloc(total);
	if(buf == NULL)
		return -1;

	long file_end = lseek(db->active_fd, 0, SEEK_END);
	long value_pos = file_end + 20 + key_len;

	pack_u64(buf+4, time(NULL));
	pack_u32(buf+12, key_len);
	pack_u32(buf+16, value_len);
	memcpy(buf+20, key, key_len);
	memcpy(buf+20+key_len, value, value_len);

	uint32_t crc = crc32_of(buf+4, body_without_crc);
	pack_u32(buf, crc);

	size_t written = 0;
	while(written < total) {
		ssize_t n = write(db->active_fd, buf+written, total-written);
		if(n < 0) {
			free(buf);
			return -1;
		}
		else if(n == 0) {
			perror("nothing was written");
			free(buf);
			return -1;
		}
		written += n;
	}

	// for (size_t i = 0; i < total; i++) {
	// 	printf("%u ", buf[i]);
	// 	printf("%02X ", buf[i]);
	// }

	free(buf);
	return value_pos;
}