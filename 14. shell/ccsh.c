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
int hc = 0, hsp = 0;

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
    
    hc = 0;
    if(fp != NULL) {
        char line[1024];
        while (fgets(line, sizeof(line), fp) != NULL)
            history[hc++] = strdup(line);
        free(h_path);
    }
    hsp = hc;
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

void free_args(char **args)
{
    if(args == NULL) return;
    for(int i = 0; args[i] != NULL; i++)
        free(args[i]);
    free(args);
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
        for(int i = 0; i < hc; i++)
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
    char **segments = malloc(16 * sizeof(char *));
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
    // disable signal generation (ISIG) so CTRL-C is read as the byte 0x03
    //   instead of the kernel raising SIGINT
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void redraw_line(const char *buf, int len)
{
    write(STDOUT_FILENO, "\r", 1);          // back to column 0
    write(STDOUT_FILENO, "ccsh>", 5);       // prompt
    write(STDOUT_FILENO, "\033[K", 3);      // clear from cursor to end of line
    if(len > 0)
        write(STDOUT_FILENO, buf, len);     // current line text
}

int load_hist_into(char *buf, const char *item)
{
    int n = strlen(item);
    if(n > 0 && item[n - 1] == '\n') n--;
    memcpy(buf, item, n);
    buf[n] = '\0';
    return n;
}

char *_fgets(char *buf, int sz, int fd)
{
    enable_raw_mode();

    char draft[1024]; // line typed before the first UP
    int draft_len = 0;
    int hpos = hc;

    int len = 0;
    char c;

    while(1) {
        ssize_t n = read(fd, &c, 1);
        if (n <= 0) {
            disable_raw_mode();
            return NULL;
        }

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

        /* CTRL-C (0x03): abandon the current line and refresh the prompt.
         *   - print ^C and a fresh prompt, like a real shell
         *   - reset the line buffer and the history-navigation state */
        if(c == 3) {
            write(STDOUT_FILENO, "^C", 2);
            write(STDOUT_FILENO, "\nccsh>", 6);
            len = 0;
            draft_len = 0;
            hpos = hc;
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
                    case 'A': // UP
                        if(hpos > 0) {
                            if(hpos == hc) {        /* leaving the draft: save what's typed */
                                memcpy(draft, buf, len);
                                draft_len = len;
                            }
                            hpos--;
                            len = load_hist_into(buf, history[hpos]);
                            redraw_line(buf, len);
                        }
                        break;
                    
                    case 'B': // DOWN
                        if(hpos < hc) {
                            hpos++;
                            if(hpos == hc) { /* back to the draft slot */
                                memcpy(buf, draft, draft_len);
                                len = draft_len;
                                buf[len] = '\0';
                            } else {
                                len = load_hist_into(buf, history[hpos]);
                            }
                            redraw_line(buf, len);
                        }
                        break;
                    
                    case 'C':
                    case 'D':
                        break;
                }
            }
            continue;
        }

        buf[len++] = c;

        if(c != '\n') write(STDOUT_FILENO, &c, 1);
    }
    buf[len] = '\0';
    write(STDOUT_FILENO, "\n", 1);

    disable_raw_mode();
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

            char **args = build_args(cmd);
            if(is_cd_cmd(args)) {
                /* cd is a builtin: a child's chdir can't affect the shell,
                 * so it must run here, in the parent process. */
                exec_cd(args[1]);
            } else {
                char **segments = extract_segments(cmd);
                handle_cmd(segments);
                free(segments);
            }
            free_args(args);

            free(cmd);
        }
        write(1, "\nccsh>", 6);
    }
}

int main()
{
    history_load();
    signal(SIGINT, SIG_IGN);
    process_input();
    return 0;
}

// step-6
// cat ./data/t.txt | uniq | wc -l