/*
 * stdout_buffering.c  —  reproduces the "printf output disappears" problem.
 * --------------------------------------------------------------------------
 *
 * WHAT THIS SHOWS
 *   The program prints a line every second and then loops FOREVER, exactly
 *   like a server that never exits. The C library ("stdio") decides how to
 *   buffer stdout based on what stdout is connected to:
 *
 *       connected to a TERMINAL  ->  LINE buffered   (flush on every '\n')
 *       connected to a PIPE/FILE ->  FULL buffered   (flush only when the
 *                                                     ~4KB buffer fills, or
 *                                                     on a clean exit())
 *
 *   Since this program never exits, "full buffered" means the output is
 *   trapped in memory and you see nothing.
 *
 * HOW TO REPRODUCE — try it three ways:
 *
 *   1) ./stdout_buffering              stdout = TERMINAL
 *      => you SEE a new line every second  (line buffered)      [WORKS]
 *
 *   2) ./stdout_buffering | cat        stdout = PIPE
 *      => you see NOTHING                 (full buffered)       [BUG]
 *
 *   3) ./stdout_buffering > out.txt    stdout = FILE
 *      => out.txt stays EMPTY             (full buffered)       [BUG]
 *
 *   Stop it with Ctrl-C. Being killed by a signal does NOT flush the stdio
 *   buffer, so in cases 2 and 3 the trapped output is simply lost.
 *
 * THE FIX (we'll do this next):
 *   Put   setvbuf(stdout, NULL, _IOLBF, 0);   at the top of main() to force
 *   line buffering regardless of where stdout goes.
 */
#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("booting up\n");          /* you'd expect to see this immediately */

    for (int i = 1;; i++) {
        printf("working... %d\n", i); /* ...and this every second */
        sleep(1);
    }
    return 0;   /* never reached — so a clean-exit flush never happens */
}
