#include<stdio.h>
#include <string.h>
#include <assert.h>
#include "../diff_tool.h"

void test_lcs_word()
{
    assert(strcmp("ABCDEF", lcs_word("ABCDEF", "ABCDEF")) == 0);
    assert(strcmp("", lcs_word("ABC", "XYZ")) == 0);
    assert(strcmp("XY", lcs_word("AABCXY", "XYZ")) == 0);
    assert(strcmp("", lcs_word("", "")) == 0);
    assert(strcmp("AC", lcs_word("ABCD", "AC")) == 0);
}