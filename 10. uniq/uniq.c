#include <stdio.h>
#include <string.h>

#define MAX_LEN 10000

int main(int argc, char *argv[])
{
    FILE *fp;

    if(argc > 1)
        fp = fopen(argv[1], "r");
    else
        fp = stdin;

    char buf[MAX_LEN] = {0};
    char prev[MAX_LEN] = {0};
    while((fgets(buf, sizeof(buf), fp)) != NULL) {
        if(strcmp(buf, prev) != 0)
            printf("%s", buf);
        strcpy(prev, buf);
    }

    return 0;
}