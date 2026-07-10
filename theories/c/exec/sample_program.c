#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("Arguments passed: %5d\n", argc);
    for(int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }
    printf("pid inside sample_program: %d\n", getpid());
    return 0;
}
// gcc sample_program.c -o sample_program