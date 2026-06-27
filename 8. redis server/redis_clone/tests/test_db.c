/*
 * Test harness for db.c (install / lookup / undef).
 *
 * Quick-test trick: we #include "db.c" directly so the static `hashtab`
 * and the `Entry` type are visible. Later, when you split a proper db.h out,
 * swap this for #include "db.h" and compile db.c + test_db.c together.
 *
 * Build:
 *   gcc -Wall -Wextra -g -fsanitize=address src/test_db.c -o src/test_db && ./src/test_db
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/db.c"
#include "../src/server.h"

static int failures = 0;

#define CHECK(cond, msg) do {                              \
    if (cond) { printf("  PASS  %s\n", msg); }             \
    else      { printf("  FAIL  %s\n", msg); failures++; } \
} while (0)

/* convenience: get the value string for a key, or NULL */
static const char *val(const char *key)
{
    Entry *e = lookup(key);
    return e ? e->value : NULL;
}

/* ---- install / lookup ------------------------------------------------- */

static void test_install_and_lookup(void)
{
    printf("[install + lookup]\n");
    install("name", "John", DEFAULT_EXPIRES_AT_MS);
    CHECK(val("name") && strcmp(val("name"), "John") == 0, "single key stored");

    install("city", "Dhaka", DEFAULT_EXPIRES_AT_MS);
    install("lang", "C", DEFAULT_EXPIRES_AT_MS);
    CHECK(val("city") && strcmp(val("city"), "Dhaka") == 0, "second key stored");
    CHECK(val("lang") && strcmp(val("lang"), "C") == 0,     "third key stored");
    CHECK(val("missing-key") == NULL, "missing key returns NULL");
}

static void test_update(void)
{
    printf("[update existing key]\n");
    install("upd", "1", DEFAULT_EXPIRES_AT_MS);
    install("upd", "2", DEFAULT_EXPIRES_AT_MS);
    install("upd", "3", DEFAULT_EXPIRES_AT_MS);
    CHECK(val("upd") && strcmp(val("upd"), "3") == 0,
          "repeated install updates value, not duplicates the node");
}

static void test_collisions(void)
{
    printf("[collisions / chaining]\n");
    /* 3000 keys into 1000 buckets => guaranteed collisions by pigeonhole.
       If chaining works, every key must still be retrievable. */
    char key[32], v[32];
    for (int i = 0; i < 3000; i++) {
        snprintf(key, sizeof(key), "key:%d", i);
        snprintf(v,    sizeof(v),    "val:%d", i);
        install(key, v, DEFAULT_EXPIRES_AT_MS);
    }
    int ok = 1;
    for (int i = 0; i < 3000; i++) {
        snprintf(key, sizeof(key), "key:%d", i);
        snprintf(v,    sizeof(v),    "val:%d", i);
        const char *got = val(key);
        if (!got || strcmp(got, v) != 0) { ok = 0; break; }
    }
    CHECK(ok, "3000 keys (many colliding) all retrievable");
}

/* ---- undef ------------------------------------------------------------ */

static void test_undef_basic(void)
{
    printf("[undef basic]\n");
    install("keep", "yes", DEFAULT_EXPIRES_AT_MS);
    install("gone", "no", DEFAULT_EXPIRES_AT_MS);
    CHECK(val("gone") != NULL, "key exists before undef");

    undef("gone");
    CHECK(val("gone") == NULL, "key gone after undef");

    undef("never-existed");           /* must not crash */
    CHECK(val("keep") != NULL, "unrelated key still intact after removing missing key");
}

/* The two tests below are the ones that catch the chain-corruption bug:
   removing a non-head node must not orphan the nodes before it. */

static void test_undef_kept_keys_survive(void)
{
    printf("[undef keeps surviving keys]\n");
    char key[32], v[32];
    int N = 3000;
    for (int i = 0; i < N; i++) {
        snprintf(key, sizeof(key), "del:%d", i);
        snprintf(v,    sizeof(v),    "val:%d", i);
        install(key, v, DEFAULT_EXPIRES_AT_MS);
    }
    /* remove every even key; many will be mid-chain */
    for (int i = 0; i < N; i += 2) {
        snprintf(key, sizeof(key), "del:%d", i);
        undef(key);
    }
    int ok = 1;
    for (int i = 0; i < N; i++) {
        snprintf(key, sizeof(key), "del:%d", i);
        const char *got = val(key);
        if (i % 2 == 0) {
            if (got != NULL) ok = 0;                 /* removed -> must be gone */
        } else {
            snprintf(v, sizeof(v), "val:%d", i);
            if (!got || strcmp(got, v) != 0) ok = 0; /* kept -> must survive */
        }
    }
    CHECK(ok, "removed keys gone, kept keys intact (no chain corruption)");
}

int run_db_tests(void)
{
    failures = 0;
    test_install_and_lookup();
    test_update();
    test_collisions();
    test_undef_basic();
    test_undef_kept_keys_survive();

    if (failures == 0) {
        printf("[db] ALL TESTS PASSED\n");
        return 0;
    }
    printf("[db] %d CHECK(S) FAILED\n", failures);
    return failures;
}


// gcc -Wall -Wextra -g -fsanitize=address tests/test_db.c -o ./tests/a && ./tests/a && rm ./tests/a