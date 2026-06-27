#ifndef DB_H
#define DB_H

void db_set(const char *key, const char *value);
void db_set_ex(const char *key, const char *value, uint64_t expires_at_ms);
char *db_get(const char *key);

#endif