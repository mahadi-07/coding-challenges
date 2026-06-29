#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../lib/algorithm.h"

int main()
{
    assert(strcmp("ABCDEF", lcs("ABCDEF", "ABCDEF")) == 0);
    assert(strcmp("", lcs("ABC", "XYZ")) == 0);
    assert(strcmp("XY", lcs("AABCXY", "XYZ")) == 0);
    assert(strcmp("", lcs("", "")) == 0);
    assert(strcmp("AC", lcs("ABCD", "AC")) == 0);

    return 0;
}