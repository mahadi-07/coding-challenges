#ifndef DB_DTO_H
#define DB_DTO_H

#include <stdbool.h>
#include <stdint.h>

/**
 * A single key-value entry stored in the database.
 *
 * Both `key` and `value` are heap-allocated and owned by the entry.
 * `expires_at_ms` is an absolute Unix timestamp in milliseconds. A value of
 * 0 indicates that the entry does not expire.
 */
typedef struct Entry Entry;
struct Entry
{
    char *key;
    char *value;
    uint64_t expires_at_ms;
    struct Entry *next;
};

/**
 * Result of a database operation.
 *
 * Ownership:
 *   - On success, the caller owns `value` and must free it.
 *   - On failure, `error` points to a static string and must not be freed.
 *
 * Invariants:
 *   - `ok == true`  => `value != NULL`, `error == NULL`
 *   - `ok == false` => `value == NULL`, `error != NULL`
 *
 * Since `value` is guaranteed to be NULL on failure, callers may safely call
 * `free(result.value)` without checking `ok`.
 */
typedef struct
{
    bool ok;
    char *value;
    const char *error;
} DbResult;

#endif
