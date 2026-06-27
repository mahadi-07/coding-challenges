#ifndef DB_H
#define DB_H

/*
 * db.h -- Public API for the in-memory key/value store.
 *
 * All functions are thread-safe.
 *
 * Ownership:
 *   - Input strings are borrowed for the duration of the call.
 *   - Strings returned by this API are heap-allocated and owned by the caller.
 *     The caller is responsible for freeing them with free().
 *
 * Expiration:
 *   - Expired keys are removed lazily when accessed.
 */
#include <stdint.h>
#include "db_dto.h"

/**
 * @brief Store a value without an expiration.
 *
 * Associates `value` with `key`, replacing any existing value and removing
 * any expiration previously associated with the key.
 *
 * @param key   Key to store.
 * @param value Value to associate with the key.
 */
void db_set(const char *key, const char *value);

/**
 * @brief Store a value with an expiration time.
 *
 * Associates `value` with `key`, replacing any existing value and expiration.
 *
 * @param key            Key to store.
 * @param value          Value to associate with the key.
 * @param expires_at_ms  Absolute Unix timestamp in milliseconds at which the
 *                       key expires. A value of 0 disables expiration.
 */
void db_set_ex(const char *key, const char *value, uint64_t expires_at_ms);

/**
 * @brief Retrieve the value associated with a key.
 *
 * Returns a newly allocated copy of the stored value, or NULL if the key does
 * not exist or has expired.
 *
 * @param key Key to look up.
 *
 * @return A heap-allocated copy of the value, or NULL if the key is absent.
 */
char *db_get(const char *key);

/**
 * @brief Remove a key from the database.
 *
 * @param key Key to remove.
 *
 * @return true if the key existed and was removed; otherwise false.
 */
bool db_del(const char *key);

/**
 * @brief Increment the integer value stored at a key.
 *
 * If the key does not exist, it is treated as 0 and the resulting value is 1.
 * If the existing value is not a valid base-10 integer, the operation fails
 * without modifying the stored value.
 *
 * @param key Key to increment.
 *
 * @return A heap-allocated DbResult describing the outcome. On success,
 *         DbResult.value contains the updated value.
 */
DbResult *db_incr(const char *key);

#endif