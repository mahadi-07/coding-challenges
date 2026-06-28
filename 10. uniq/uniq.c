#include <stdio.h>
#include <string.h>

#define MAX_LEN 10000

int main(int argc, char *argv[])
{
    FILE *fp = stdin;
    FILE *fp_out = stdout;

    int print_count = 0;
    int repeated_lines_only = 0;
    int unique_lines_only = 0;
    char *input_file_path = NULL;
    char *output_file_path = NULL;

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--count") == 0)
            print_count = 1;
        else if(strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--repeated") == 0)
            repeated_lines_only = 1;
        else if(strcmp(argv[i], "-u") == 0) 
            unique_lines_only = 1;
        else if(strcmp(argv[1], "-") == 0) {
            if (i + 1 < argc)
                output_file_path = argv[++i];
        }
        else {
            /* First non-option is the input file. */
            if (input_file_path == NULL)
                input_file_path = argv[i];
        }
    }


    if (input_file_path != NULL) {
        fp = fopen(input_file_path, "r");
        if (fp == NULL) {
            perror(input_file_path);
            return 1;
        }
    }

    if (output_file_path != NULL) {
        fp_out = fopen(output_file_path, "wb");
        if (fp_out == NULL) {
            perror(output_file_path);
            fclose(fp);
            return 1;
        }
    }

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
            if(print_count)
                printf("%3d %s", count, prev);
            else
                printf("%s", prev);
        }
    }

    if(fp != NULL) fclose(fp);
    if(fp_out != NULL) fclose(fp_out);

    return 0;
}