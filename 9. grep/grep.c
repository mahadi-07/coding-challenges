#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
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
                    printf("%s -> %s", file_path, buf);
            }
        }
    }

    closedir(dir);
    return 0;
}

int main(int argc, char *argv[])
{

    // list_lines_recursively("./data", "Nirvana");

    // exit(1);


    if(argc < 3) {
        fprintf(stderr, "Invalid number of args\n");
        exit(1);
    }

    char *pattern = argv[1];

    if(strcasecmp(pattern, "-r") == 0) {
        list_lines_recursively(argv[3], argv[2]);
        return 0;
    }

    char *file_path = argv[2];

    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        perror("fopen failed");
        exit(1);
    }

    char buf[MAX_LINE_LENGTH];

    enum Type type = ALL;
    int matched_line_count = 0;
    while((fgets(buf, sizeof(buf), fp)) != NULL) {
        if(pattern[0] == '\0') {
            type = ALL;
            printf("%s", buf);
        }
        else if(strlen(pattern) == 1) {
            type = SINGLE_CHARACTER;
            int sz = strlen(buf);
            for(int i = 0; i < sz; i++) {
                if(buf[i] == pattern[0]) {
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