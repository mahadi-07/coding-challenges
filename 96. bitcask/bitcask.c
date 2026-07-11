#include <stdio.h>
#include <stdlib.h>
#include "db.h"

int main()
{

    bitcask_t *db = bitcask_open("./database", 100);
    // printf("%s %d %d %zu\n", db->dir, db->active_fd, db->active_file_id, db->threshold);

    bitcask_append(db, "shakil", "flksdjflkajsdklfjasdkf");
    // bitcask_append(db, "11", "fklsjdklffjlksdf");

    bitcask_close(db);
    exit(0);



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