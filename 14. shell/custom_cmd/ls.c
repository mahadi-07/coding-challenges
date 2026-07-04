#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

int ls(const char *path)
{
    DIR *dir = opendir(path);
    if(dir == NULL)
        return 1;

    struct dirent *entity;
    while((entity = readdir(dir)) != NULL) {
        if(strcasecmp(entity->d_name, ".") == 0 || strcasecmp(entity->d_name, "..") == 0) 
            continue;

        if(entity->d_type == DT_DIR) {
            char *nw_path = malloc(sizeof(100) * sizeof(char));
            snprintf(nw_path, sizeof(nw_path), "%s/%s", path, entity->d_name);
            ls(nw_path);
            
            free(nw_path);
        }
        else
            printf("%s\t", entity->d_name);
    }

    return 0;
}

int main()
{
    ls(".");

    printf("\n");
}