#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>

#define MAX_HISTORY 1000

char *history[MAX_HISTORY] = {0};
int hidx = 0, h_scroll = 0;

char *get_h_path()
{
    char h_path[100];

    snprintf(h_path, sizeof(h_path),
             "%s/%s",
             getenv("HOME"),
             ".ccsh_history");
    return strdup(h_path);
}

void history_load()
{
    char *h_path = get_h_path();
    FILE *fp = fopen(h_path, "r");
    
    hidx = 0;
    if(fp != NULL) {
        char line[1024];
        while (fgets(line, sizeof(line), fp) != NULL)
            history[hidx++] = strdup(line);
        free(h_path);
    }
    h_scroll = hidx;
}

int get_hfd()
{
    char *h_path = get_h_path();
    int hfd = open(h_path, O_RDWR | O_CREAT | O_APPEND, 0644);
    free(h_path);
    return hfd;
}

void history_add(const char *cmd)
{
    int hfd = get_hfd();
    lseek(hfd, 0, SEEK_END);
    write(hfd, cmd, strlen(cmd));
    write(hfd, "\n", 1);
    close(hfd);

    history_load();
}

void unlink_ccsh_history()
{
    char *h_path = get_h_path();
    unlink(h_path);
    free(h_path);
}

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

    if(strcasecmp(args[0], "history") == 0) {
        int seq_no = 1;
        for(int i = 0; i < hidx; i++)
            printf("%5d\t%s", seq_no++, history[i]);
        return;
    }

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
            signal(SIGINT, SIG_DFL);

            if(i > 0) dup2(pipefds[i-1][0], STDIN_FILENO);
            if(i < sc-1) dup2(pipefds[i][1], STDOUT_FILENO);

            for(int j = 0; j < sc; j++) {
                close(pipefds[j][0]); close(pipefds[j][1]);
            }

            execute_cmd(segments[i]);
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

/**
 * - Turning on raw mode
 * - Reading one character at a time
 */

struct termios original;

/* Restore the terminal before existing */
void disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
}

/* Enable raw mode */
void enable_raw_mode()
{
    tcgetattr(STDIN_FILENO, &original);

    struct termios raw = original;

    // disable canonical mode (dont wait for Enter)
    // disable echo (terminal won't print characters)
    raw.c_lflag &= ~(ECHO | ICANON);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

char *_fgets(char *buf, int sz, int fd)
{
    int len = 0;
    char c;

    while(1) {
        ssize_t n = read(fd, &c, 1);
        if (n <= 0) 
            return NULL;

        /* user pressed enter */
        if(c == '\n' || len >= sz)
            break;

        /* backspace (127 on most terminals) */
        if(c == 127) {
            if(len > 0) {
                len--;
                // move left, erase character, move left again
                write(STDOUT_FILENO, "\b \b", 3);
            }
            continue;
        }

        if(c == 27) {
            char second;
            char third;

            read(STDIN_FILENO, &second, 1);
            read(STDIN_FILENO, &third, 1);

            if(second == '[') {
                switch (third)
                {
                    case 'A':
                        if(h_scroll > 0) {
                            --h_scroll;
                            write(1, history[h_scroll], strlen(history[h_scroll]));
                        }
                        break;
                    
                    case 'B':
                        printf("\nDOWN\n");
                        break;
                    
                    case 'C':
                        printf("\nRIGHT\n");
                        break;

                    case 'D':
                        printf("\nLEFT\n");
                        break;
                }
            }
            continue;
        }

        buf[len++] = c;

        write(STDOUT_FILENO, &c, 1);
    }
    buf[len] = '\0';
    write(STDOUT_FILENO, "\n", 1);

    return buf;
}

void process_input()
{
    write(1, "\nccsh>", 6);
    char buf[1024] = {};
    while(_fgets(buf, sizeof(buf), 0) != NULL) {
        char *cmd = trim(buf);
        if(cmd != NULL) {            
            history_add(cmd);

            if(strcasecmp(cmd, "exit") == 0)
                break;

            char **segments = extract_segments(cmd);
            handle_cmd(segments);
            
            free(cmd);
            free(segments);
        }
        write(1, "\nccsh>", 6);
    }
}

int main()
{
    history_load();
    signal(SIGINT, SIG_IGN);

    enable_raw_mode();
    process_input();
    disable_raw_mode();
    return 0;
}

// step-6
// cat ./data/t.txt | uniq | wc -l