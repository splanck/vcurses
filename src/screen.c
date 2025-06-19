#include "vcurses.h"

#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

static struct termios saved_termios;
static int term_saved = 0;

int vc_init(void)
{
    const char *term = getenv("TERM");
    if (!term || !*term)
        return -1;

    if (!isatty(STDIN_FILENO))
        return -1;

    if (tcgetattr(STDIN_FILENO, &saved_termios) < 0)
        return -1;

    struct termios raw = saved_termios;
    cfmakeraw(&raw);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0)
        return -1;

    term_saved = 1;
    return 0;
}

void vc_end(void)
{
    if (term_saved) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_termios);
        term_saved = 0;
    }
}

