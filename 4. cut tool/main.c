#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 1000

int main(int argc, char *argv[])
{
    if(argc < 3) {
        perror("File path is missing.");
        exit(1);
    }

    char *filename = *++argv;
    if(filename[0] == '-' && filename[1] == 'f') {
        int col = atoi(filename + 2);
        if(col > 0) {
            FILE *fp = fopen(*++argv, "r");

            char buf[BUFSIZE];
            while(fgets(buf, BUFSIZE, fp) != NULL) {
                char *p = buf;
                int field = 1;
                while(*p) {
                    if(*p == ' ' || *p == '\t' || *p == '\n')
                        field++;
                    else {
                        if(field == col) printf("%c", *p);
                    }
                    p++;
                }
                printf("\n");
            }
        }
    }

    exit(0);
}

// gcc main.c -o cut && ./cut data/sample.tsv && rm cut
// gcc main.c -o cut && ./cut -f2 data/sample.tsv && rm cut
// gcc main.c -o cut && ./cut -f2 data/sample.tsv | head -n5 && rm cut