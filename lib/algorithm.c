#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "algorithm.h"

char *lcs(const char *a, const char *b)
{
    const int LEN = 1001;

    int dp[LEN][LEN];
    memset(dp, 0, sizeof(dp));

    int n = strlen(a);
    int m = strlen(b);

    for(int i = 1; i <= n; i++){
        for(int j = 1; j <= m; j++) {
            if(a[i-1] == b[j-1])
                dp[i][j] = dp[i-1][j-1] + 1;
            else if(dp[i-1][j] >= dp[i][j-1])
                dp[i][j] = dp[i-1][j];
            else
                dp[i][j] = dp[i][j-1];
        }
    }

    char *c = malloc(dp[n][m] + 1);
    int idx = dp[n][m];
    c[idx--] = '\0';

    int i = n, j = m;

    while(i > 0 && j > 0) {
        if(a[i-1] == b[j-1]) {
            c[idx--] = a[i-1];
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