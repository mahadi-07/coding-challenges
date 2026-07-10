/**
 * Word
 */
typedef struct {
    const char *a;
    const char *b;
} word_ctx;

static bool word_equal(int i, int j, void *ctx)
{
    word_ctx *w = (word_ctx *) ctx;
    return w->a[i] == w->b[j];
}

/**
 * Lines
 */
typedef struct {
    char **items;
    int count;
} str_list;

typedef struct diff_tool
{
    str_list a;
    str_list b;
} line_ctx;

static bool line_equal(int i, int j, void *ctx)
{
    line_ctx *l = (line_ctx *) ctx;
    return strcmp(l->a.items[i], l->b.items[j]) == 0;
}

/**
 * methods
 */
char *lcs_word(const char *a, const char *b);

str_list lcs_lines(str_list a, str_list b);