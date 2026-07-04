#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main()
{
    char cwd[1000];
    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("pwd error");
        return 1;
    }
    write(STDOUT_FILENO, cwd, strlen(cwd));
}