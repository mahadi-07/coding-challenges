#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#define BUFSIZE 1000

int main(int argc, char *argv[])
{
    if(argc < 3) {
        perror("File path is missing.");
        exit(1);
    }

    char delimiter = '\t';
    unsigned int col_fetch = 0;
    int cval = 0;
    char *filename = NULL;

    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-' && argv[i][1] == 'f') {
            char *carg = argv[i] + 2;
            while(*carg) {
                if(*carg == ' ' || *carg == ',') {
                    col_fetch |= (1u << cval);
                    cval = 0;
                }
                else if(!isdigit((unsigned char) *carg)) {
                    fprintf(stderr, "provided cmd contain invalid values: %s\n", *argv);
                    exit(1);
                }
                else
                    cval = cval * 10 + (*carg - '0');
                carg++;
            }
            col_fetch |= (1u << cval);
        }
        else if(argv[i][0] == '-' && argv[i][1] == 'd')
            delimiter = *(argv[i] + 2);
        else
            filename = argv[i];
    }

   if(col_fetch > 0) {
        FILE *fp = fopen(filename, "r");
        if(fp == NULL) {
            fprintf(stderr, "error opending file: %s\n", filename);
            exit(1);
        }

        char buf[BUFSIZE];
        while(fgets(buf, BUFSIZE, fp) != NULL) {
            char *p = buf;
            int field = 1;
            while(*p) {
                if(delimiter == *p) {
                    field++;
                    if(*p != '\n' && (col_fetch & (1 << field)))
                        putchar(delimiter);
                }
                else 
                    if(col_fetch & (1 << field)) printf("%c", *p);

                p++;
            }
            putchar('\n');
        }
    }

    exit(0);
}

/* step 1: gcc main.c -o cut && ./cut -f2 data/sample.tsv && rm cut */
// gcc main.c -o cut && ./cut -f5 data/sample.tsv | cat -e && rm cut

/* step 2: gcc main.c -o cut && ./cut -f1 -d, data/fourchords.csv | head -n5 && rm cut */

/* step 3 */
/* gcc main.c -o cut && ./cut -f"1 2, 3, 4" data/sample.tsv && rm cut */
/* gcc main.c -o cut && ./cut -f1,2,3,4 -d, data/fourchords.csv | head -n5 && rm cut */
// gcc main.c -o cut && ./cut -d, -f1,2,3,4 data/fourchords.csv | head -n5 && rm cut

// gcc main.c -o cut && ./cut data/sample.tsv && rm cut
// gcc main.c -o cut && ./cut -f2 data/sample.tsv && rm cut
// gcc main.c -o cut && ./cut -f2 data/sample.tsv | head -n5 && rm cut