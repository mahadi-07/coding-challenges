#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("pid inside execvp: %10d\n", getpid());

    char *prgm = "./sample_program";
    char *args[] = {prgm, "1", "3", "5", NULL};
    execvp(prgm, args);
    
    printf("Will not print, as on success execvp() never returns because \
            the current process image is replaced");

    // execvp returns -1 only when it fails (e.g. ./sample_program is missing or not executable)

    return 0;
}