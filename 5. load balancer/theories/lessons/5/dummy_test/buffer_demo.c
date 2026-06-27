#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Usage: ./buffer_demo [default|full|line|none]
//   default -> let stdio choose (FULL buffering when output is a pipe/file)
//   full    -> force FULL buffering
//   line    -> force LINE buffering (flush every '\n')  [what a terminal does]
//   none    -> NO buffering (flush every char)
int main(int argc, char *argv[]) {
    const char *mode = (argc > 1) ? argv[1] : "default";
    if      (strcmp(mode, "line") == 0) setvbuf(stdout, NULL, _IOLBF, 0);
    else if (strcmp(mode, "none") == 0) setvbuf(stdout, NULL, _IONBF, 0);
    else if (strcmp(mode, "full") == 0) setvbuf(stdout, NULL, _IOFBF, 0);
    /* "default" -> leave stdio's automatic decision */

    printf("started (%s)\n", mode);
    for (int i = 1; i <= 3; i++) {
        printf("tick %d\n", i);
        sleep(1);
    }
    for (;;) sleep(1);   // loop forever, like your server's while(1)
    return 0;
}
