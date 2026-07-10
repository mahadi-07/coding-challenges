#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

/**
 * not re-entrant, because of this static method
 * 
 * Guaranteeing that your functions are signal handler safe can’t be solved by 
 * removing shared buffers. You must also think about multithreading and 
 * synchronization – what happens when I double lock a mutex? You also have 
 * to make sure that each function call is reentrant safe. Suppose your original 
 * program was interrupted while executing the library code of malloc. 
 * The memory structures used by malloc will be inconsistent. Calling printf, 
 * which uses malloc as part of the signal handler, is unsafe and will result 
 * in undefined behavior. 
 * 
 * A safe way to avoid this behavior is to set a variable and let the program 
 * resume operating. The design pattern also helps us in designing programs that 
 * can receive signals twice and operate correctly.
 */
int func(const char *str)
{
    static char buffer[200];

    strncpy(buffer, str, 199);
    
    buffer[sizeof(buffer) - 1] = '\0';
    printf("Copied \"%s\" into buffer\n", buffer);

    // Here is where we get pasued
    if (strcmp(str, "Hello") == 0)
    {
        printf("Raising SIGUSR1 before printing...\n\n");
        raise(SIGINT);
    }

    printf("%s\n", buffer);
    return 1;
}

void handler(int sig)
{
    printf("\n[Signal Handler] Interrupt received!\n");
    func("World");
}

int main()
{
    signal(SIGINT, handler);

    func("Hello");

    return 0;
}