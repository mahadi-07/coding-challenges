#include <stdio.h>
#include "db.h"

int main()
{
    char *db_path = "./database/cask.0";
    hashtable_t *ht = ht_new();

    db_load(ht, db_path);

    // ht_set(ht, "1", "world");
    ht_set(ht, "1", "world111");

    const char *q1 = ht_get(ht, "1");
    if(q1 != NULL)
        printf("Value: %s\n", q1);
    else
        printf("key not found");

    db_save(ht, db_path);

    ht_set(ht, "2", "world111");
    db_save(ht, db_path);

    return 0;
}