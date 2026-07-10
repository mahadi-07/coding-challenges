#include <stdio.h>
#include <unistd.h>
#include <termios.h>

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

int main()
{
    char buf[1024];
    int len = 0;
    char c;

    enable_raw_mode();

    printf("Type something: ");
    fflush(stdout);
    
    while(1) {
        read(STDIN_FILENO, &c, 1);

        /* user pressed enter */
        if(c == '\n')
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
                        printf("\nUP\n");
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

    disable_raw_mode();

    printf("\n\nResult = %s\n", buf);
}