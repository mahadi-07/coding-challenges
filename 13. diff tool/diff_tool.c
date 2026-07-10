#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> 
#include "diff_tool.h"
    
const int LEN = 1001;

typedef bool (*cmp_fn) (int i, int j, void *ctx);

static void build_lcs_table(
    void **a,
    int dp[LEN][LEN],
    int n,
    int m,
    cmp_fn equal,
) {

    for(int i = 1; i <= n; i++){
        for(int j = 1; j <= m; j++) {
            if(cmp_fn())
                dp[i][j] = dp[i-1][j-1] + 1;
            else if(dp[i-1][j] >= dp[i][j-1])
                dp[i][j] = dp[i-1][j];
            else
                dp[i][j] = dp[i][j-1];
        }
    }

    char *c = malloc(sizeof(dp[n][m] + 1));
    int idx = dp[n][m];
    int i = n, j = m;

    while(i > 0 && j > 0) {
        if(a[i-1] == b[j-1]) {
            c[--idx] = a[i-1];
            i--;
            j--;
        }
        else if(dp[i-1][j] > dp[i][j-1])
            i--;
        else
            j--;
    }
    
    return c;
}


str_list lcs_lines(str_list a, str_list b)
{
    const int LEN = 1001;

    int dp[LEN][LEN];
    memset(dp, 0, sizeof(dp));

    int n = a.count;
    int m = b.count;

    for(int i = 1; i <= n; i++){
        for(int j = 1; j <= m; j++) {
            if(strcmp(a.items[i-1], b.items[j-1]) == 0)
                dp[i][j] = dp[i-1][j-1] + 1;
            else if(dp[i-1][j] >= dp[i][j-1])
                dp[i][j] = dp[i-1][j];
            else
                dp[i][j] = dp[i][j-1];
        }
    }

    str_list result;
    result.count = dp[n][m];
    result.items = malloc(sizeof(char *) * (dp[n][m] + 1));

    int idx = dp[n][m];
    int i = n, j = m;

    while(i > 0 && j > 0) {
        if(strcmp(a.items[i-1], b.items[j-1]) == 0) {
            result.items[--idx] = a.items[i-1];
            i--;
            j--;
        }
        else if(dp[i-1][j] > dp[i][j-1])
            i--;
        else
            j--;
    }
    
    return result;
}