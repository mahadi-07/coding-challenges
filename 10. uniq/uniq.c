#include <stdio.h>
#include <string.h>

#define MAX_LEN 10000

int main(int argc, char *argv[])
{
    FILE *fp = NULL;
    FILE *fp_out = NULL;

    int print_count = 0;
    int repeated_lines_only = 0;
    int unique_lines_only = 0;
    if(argc > 1) {
        if(strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--count") == 0) {
            print_count = 1;
            fp = fopen(argv[2], "r");
        }
        else if(strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--repeated") == 0) {
            repeated_lines_only = 1;
            fp = fopen(argv[2], "r");
        }
        else if(strcmp(argv[1], "-u") == 0) {
            unique_lines_only = 1;
            fp = fopen(argv[2], "r");
        }
        else if(strcmp(argv[1], "-") == 0) {
            fp = stdin;
            if(argc >= 3) {
                fp_out = fopen(argv[2], "wb");
                if(fp_out == NULL)
                    return 1;
            }
        }
        else
            fp = fopen(argv[1], "r");
        
        
    }
    else
        fp = stdin;

    char buf[MAX_LEN] = {0};
    char prev[MAX_LEN] = {0};
    int count = 0;
    while((fgets(buf, sizeof(buf), fp)) != NULL) {
        if(prev[0] != '\0' && strcmp(buf, prev) != 0) {
            int should_print = 1;
            if(repeated_lines_only) should_print = count > 1;
            if(unique_lines_only) should_print = (count == 1);

            if(should_print) {
                if(fp_out == NULL) {
                    if(print_count)
                        printf("%3d %s", count, prev);
                    else
                        printf("%s", prev);
                }
                else {
                    if(print_count) {
                        char updated[2 * MAX_LEN] = {0};
                        snprintf(updated, sizeof(updated), "%3d %s", count, prev);
                        fwrite(&updated, sizeof(char), strlen(updated), fp_out);
                    }
                    else
                        fwrite(&prev, sizeof(char), strlen(prev), fp_out);
                }
            }
            
            count = 1;
        }
        else
            count++;

        strcpy(prev, buf);
    }
    if(count > 0) {
        int should_print = 1;
        if(repeated_lines_only) should_print = count > 1;
        if(unique_lines_only) should_print = (count == 1);
        if(should_print) {
            if(print_count) {
                if(count > 1) printf("%3d %s", count, prev);
            }
            else
                printf("%s", prev);
        }
    }

    if(fp != NULL) fclose(fp);
    if(fp_out != NULL) fclose(fp_out);

    return 0;
}