#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "../diff_tool.h"

static int list_eq(str_list got, char **expected, int expected_count) { 
    if(got.count != expected_count) 
        return 0;

    for(int i = 0; i < got.count; i++) 
        if (strcmp(got.items[i], expected[i])) return 0;

    return 1; 
}

void test_lcs_lines()
{
    {
        char *a[] = { "This is a test which contains:", "this is the lcs" }; 
        char *b[] = { "this is the lcs", "we're testing" }; 
        char *exp[] = { "this is the lcs" }; 
        str_list la = { a, 2 }; 
        str_list lb = { b, 2 }; 
        str_list got = lcs_lines(la, lb);
        assert(1 == list_eq(got, exp, 1));
        free(got.items);
    }


    {
        char *a[] = {
            "Coding Challenges helps you become a better software engineer through that build real applications.",
            "I share a weekly coding challenge aimed at helping software engineers level up their skills through deliberate practice.",
            "I've used or am using these coding challenges as exercise to learn a new programming language or technology.",
            "Each challenge will have you writing a full application or tool. Most of which will be based on real world tools and utilities."
        };
        char *b[] = {
            "Helping you become a better software engineer through coding challenges that build real applications.",
            "I share a weekly coding challenge aimed at helping software engineers level up their skills through deliberate practice.",
            "These are challenges that I've used or am using as exercises to learn a new programming language or technology.",
            "Each challenge will have you writing a full application or tool. Most of which will be based on real world tools and utilities."
        };
        char *exp[] = {
            "I share a weekly coding challenge aimed at helping software engineers level up their skills through deliberate practice.",
            "Each challenge will have you writing a full application or tool. Most of which will be based on real world tools and utilities."
        };
        str_list la = { a, 4 };
        str_list lb = { b, 4 };
        str_list got = lcs_lines(la, lb);
        assert(list_eq(got, exp, 2));
        free(got.items);
    }
}