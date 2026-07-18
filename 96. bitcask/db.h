#define HT_BUCKETS 1024u

typedef struct {
    char *value;
} payload_t;

typedef struct node {
    char *key;
    payload_t payload;
    struct node *next;
} note_t;

typedef struct {
    note_t *buckets[HT_BUCKETS];
} hashtable_t;

typedef struct {
    char *dir;
    int active_fd;
    int active_file_id;
    size_t threshold;
} bitcask_t;

hashtable_t *ht_new(void);
void ht_free(hashtable_t *ht);
void ht_set(hashtable_t *ht, const char *key, const char *value);
const char *ht_get(const hashtable_t *ht, const char *key);

int db_save(const hashtable_t *ht, const char *dir);
int db_load(hashtable_t *ht, const char *dir);

bitcask_t *bitcask_open(const char *dir, const size_t threshold);
void bitcask_close(bitcask_t *cask);

long bitcask_append(bitcask_t *db, const char *key, const char *value);