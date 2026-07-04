#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

char *trim_spaces_newlines(char *cmd)
{
    int l = 0;
    int r = strlen(cmd) - 1;

    while(l <= r && (cmd[l] == ' ' || cmd[l] == '\n')) 
        ++l;

    while(r >= 0 && (cmd[r] == ' ' || cmd[r] == '\n')) 
        --r;

    int len = r - l + 1;
    if(len <= 0) return NULL;

    char *trimmed = malloc(len + 1);
    memcpy(trimmed, cmd+l, len);
    trimmed[len] = '\0';
    return trimmed;
}

int cd(const char *tdir)
{
    if(tdir == NULL || strcmp(tdir, "~") == 0)
        tdir = getenv("HOME");
    
    if(strcmp(tdir, "-") == 0)
        tdir = getenv("OLDPWD");

    char cwd[1000];
    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("cd: getcwd failed");
        return 1;
    }

    if(chdir(tdir) != 0) {
        perror("cd");
        return 1;
    }

    setenv("OLDPWD", cwd, 1);
    if(getcwd(cwd, sizeof(cwd)) != NULL)
        setenv("PWD", cwd, 1);

    return 0;
}

int exec_cd(char *path)
{
    char *trimmed = trim_spaces_newlines(path);
    cd(trimmed);
    free(trimmed);

    return 0;
}