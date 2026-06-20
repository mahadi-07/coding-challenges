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
            char *curargv = *++argv;
            char delimiter = '\t'; /* default delimiter */
            
            if(curargv[0] == '-' && curargv[1] == 'd') { /* delimitar present */
                delimiter = curargv[2];    
                curargv = *++argv;
            }

            FILE *fp = fopen(curargv, "r");

            char buf[BUFSIZE];
            while(fgets(buf, BUFSIZE, fp) != NULL) {
                char *p = buf;
                int field = 1;
                while(*p) {
                    if(delimiter == *p)
                        field++;
                    else 
                        if(field == col) printf("%c", *p);

                    p++;
                }
            }
        }
    }

    exit(0);
}

/* step 1: gcc main.c -o cut && ./cut -f2 data/sample.tsv && rm cut */
// gcc main.c -o cut && ./cut -f5 data/sample.tsv | cat -e && rm cut

/* step 2: gcc main.c -o cut && ./cut -f1 -d, data/fourchords.csv | head -n5 && rm cut */



// gcc main.c -o cut && ./cut data/sample.tsv && rm cut
// gcc main.c -o cut && ./cut -f2 data/sample.tsv && rm cut
// gcc main.c -o cut && ./cut -f2 data/sample.tsv | head -n5 && rm cut