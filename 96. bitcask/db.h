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

hashtable_t *ht_new(void);
void ht_free(hashtable_t *ht);
void ht_set(hashtable_t *ht, const char *key, const char *value);
const char *ht_get(const hashtable_t *ht, const char *key);


int db_save(const hashtable_t *ht, const char *dir);
int db_load(hashtable_t *ht, const char *dir);