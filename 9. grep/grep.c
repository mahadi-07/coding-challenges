#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>

#define MAX_PATH_LENGTH 1000
#define MAX_LINE_LENGTH 1000

enum Type {
    ALL,
    SINGLE_CHARACTER
};

int list_lines_recursively(const char *dir_path, const char *pattern)
{
    DIR *dir = opendir(dir_path);
    if(dir == NULL)
        return 1;

    struct dirent *entity;
    while((entity = readdir(dir)) != NULL) {
        if(strcasecmp(entity->d_name, ".") == 0 || strcasecmp(entity->d_name, "..") == 0) continue;
        if(entity->d_type == DT_DIR) {
            char path[MAX_PATH_LENGTH] = {0};
            snprintf(path, sizeof(path), "%s/%s", dir_path, entity->d_name);
            list_lines_recursively(path, pattern);
        }
        else {
            char file_path[MAX_PATH_LENGTH] = {0};             
            snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entity->d_name);

            FILE *fp = fopen(file_path, "r");
            char buf[MAX_LINE_LENGTH];
            while((fgets(buf, sizeof(buf), fp)) != NULL) {
                if(strstr(buf, pattern) != NULL)
                    printf("%s:%s", file_path, buf);
            }
        }
    }

    closedir(dir);
    return 0;
}

int list_lines_recursively_with_exclude(const char *dir_path, const char *pattern, const char *exclude)
{
    DIR *dir = opendir(dir_path);
    if(dir == NULL)
        return 1;

    struct dirent *entity;
    while((entity = readdir(dir)) != NULL) {
        if(strcasecmp(entity->d_name, ".") == 0 || strcasecmp(entity->d_name, "..") == 0) continue;
        if(entity->d_type == DT_DIR) {
            char path[MAX_PATH_LENGTH] = {0};
            snprintf(path, sizeof(path), "%s/%s", dir_path, entity->d_name);
            list_lines_recursively_with_exclude(path, pattern, exclude);
        }
        else {
            char file_path[MAX_PATH_LENGTH] = {0};             
            snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entity->d_name);

            FILE *fp = fopen(file_path, "r");
            char buf[MAX_LINE_LENGTH];
            while((fgets(buf, sizeof(buf), fp)) != NULL) {
                if(strstr(buf, pattern) != NULL)
                    printf("%s:%s", file_path, buf);
            }
        }
    }

    closedir(dir);
    return 0;
}

int main(int argc, char *argv[])
{
    if(argc < 3) {
        fprintf(stderr, "Invalid number of args\n");
        exit(1);
    }

    FILE *fp = NULL;

    char *opt = argv[1];
    if(strcasecmp(opt, "-r") == 0) {
        list_lines_recursively(argv[3], argv[2]);
        return 0;
    }
    else if(strcasecmp(opt, "-v") == 0)
        fp = stdin;

    if(fp == NULL) fp = fopen(argv[2], "r");
    if (!fp) {
        perror("fopen failed");
        exit(1);
    }

    char buf[MAX_LINE_LENGTH];

    enum Type type = ALL;
    int matched_line_count = 0;
    while((fgets(buf, sizeof(buf), fp)) != NULL) {
        if(opt[0] == '^') {
            char *prefix = opt+1;
            if(strncmp(buf, prefix, strlen(prefix)) == 0) {
                printf("%s", buf);
            }
        }
        else if(strcmp(opt, "\\d") == 0) {
            int digit = 0;
            for(int i = 0; buf[i] != '\0'; i++) {
                if(isdigit(buf[i])) {
                    digit = 1;
                    break;
                }
            }
            if(digit) printf("%s", buf);
        }
        else if(strcmp(opt, "\\w") == 0) {
            int non_char = 0;
            for(int i = 0; buf[i] != '\0'; i++) {
                if(buf[i] == '\n' || buf[i] == ' ' || buf[i] == '\t') continue;
                if(!isalpha(buf[i])) {
                    non_char = 1;
                    break;
                }
            }
            if(!non_char) printf("%s", buf);
        }
        else if(strcmp(opt, "-v") == 0) {
            if (strstr(buf, argv[2]) == NULL) {
                printf("%s", buf);
            }
        }
        else if(opt[0] == '\0') {
            type = ALL;
            printf("%s", buf);
        }
        else if(strlen(opt) == 1) {
            type = SINGLE_CHARACTER;
            int sz = strlen(buf);
            for(int i = 0; i < sz; i++) {
                if(buf[i] == opt[0]) {
                    matched_line_count++;
                    printf("%s", buf);
                    break;
                }
            }
        }
        else {
            fclose(fp);
            printf("undefine behavior");
            return 1;
        }
    }

    fclose(fp);
    
    if(type == SINGLE_CHARACTER)
        return matched_line_count ? 0 : 1;

    return 0;
}
// grep "" data/x.txt | diff data/y.txt -

// ./grep_ "" data/x.txt | diff data/y.txt -

// ./grep_ J data/rockbands.txt
// echo $?

// ./grep_ "\w" ./data/symbols.txt

// ./grep_ ^A ./data/rockbands.txt