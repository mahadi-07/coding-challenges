#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>

extern int exec_cd(char *path);

char *trim(char *cmd)
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

char **build_args(char *cmd)
{
    char **v = malloc(sizeof(char *) * 100);
    int pos = 0;

    char buf[100];
    int idx = 0;
    size_t i;
    for(i = 0; i < strlen(cmd); i++) {
        if(cmd[i] == ' ' || cmd[i] == '\t' || cmd[i] == '\n') {
            buf[idx++] = '\0';
            v[pos++] = strdup(buf);

            idx = 0;
        }
        else buf[idx++] = cmd[i];
    }
    if(idx > 0) {
        buf[idx] = '\0';
        v[pos++] = strdup(buf);
    }
    v[pos] = NULL;

    return v;
}

int is_cd_cmd(char **args)
{
    return strcmp(args[0], "cd") == 0;
}

void execute_cmd(char *cmd)
{
    char **args = build_args(cmd);

    /**
     * cd must be executed within the main shell process.
     * A child process cannot change the current working directory of its parent.
     */
    if(is_cd_cmd(args)) {
        exec_cd(args[1]);
        return;
    }

    pid_t pid = fork();
    if(pid == 0) {
        execvp(args[0], args);
        printf("%s (%d)", strerror(errno), errno);
        exit(1);
    }
    else
        wait(NULL);
}

void handle_cmd(char **segments)
{
    int sc = 0;
    while(segments[sc] != NULL)
        sc++;

    int pipefds[sc][2];
    for(int i = 0; i < sc; i++) pipe(pipefds[i]);

    for(int i = 0; i < sc; i++) {

        pid_t pid = fork();
        if(pid == 0) {

            if(i > 0) dup2(pipefds[i-1][0], STDIN_FILENO);
            if(i < sc-1) dup2(pipefds[i][1], STDOUT_FILENO);

            for(int j = 0; j < sc; j++) {
                close(pipefds[j][0]); close(pipefds[j][1]);
            }

            char **args = build_args(segments[i]);
            execvp(args[0], args);
            perror("execvp");
            exit(1);
        }
    }

    for(int i = 0; i < sc; i++) {
        close(pipefds[i][0]); close(pipefds[i][1]);
    }
    for(int i = 0; i < sc; i++)
        wait(NULL);

    return;
}

char **extract_segments(const char *cmd)
{
    char **segments = malloc(sizeof(16) * sizeof(char *));
    int seg_count = 0;

    char part[128];
    int pos = 0;

    int sz = strlen(cmd);
    for(int i = 0; i < sz; i++) {
        if(cmd[i] == '|') {
            part[pos++] = '\0';
            pos = 0;
            segments[seg_count++] = trim(part);
        }
        else 
            part[pos++] = cmd[i];
    }

    if(pos > 0) {
        part[pos] = '\0';
        segments[seg_count++] = trim(part);
    }

    segments[seg_count] = NULL;
    return segments;
}

int main()
{
    printf("ccsh>");
    char buf[1000] = {};
    while(fgets(buf, sizeof(buf), stdin) != NULL) {
        char *cmd = trim(buf);
        if(cmd != NULL) {
            if(strcasecmp(cmd, "exit") == 0)
                break;

            char **segments = extract_segments(cmd);
            free(cmd);

            for(int i = 0; segments[i] != NULL ; i++)
                printf("[%s]\n", segments[i]);



            handle_cmd(segments);
            
            free(segments);
        }

        printf("\nccsh>");
    }
}



// step-6
// cat ./data/t.txt | uniq | wc -l