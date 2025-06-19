#include "curses.h"
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>

static struct termios orig_termios;
static int term_initialized = 0;

static void restore_terminal(void) {
    if (term_initialized) {
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
        term_initialized = 0;
    }
}

static void handle_signal(int sig) {
    restore_terminal();
    signal(sig, SIG_DFL);
    raise(sig);
}

static void handle_winch(int sig) {
    (void)sig;
    if (stdscr) {
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
            stdscr->maxy = ws.ws_row;
            stdscr->maxx = ws.ws_col;
        }
    }
}

WINDOW *initscr(void) {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        return NULL;
    }

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) {
        return NULL;
    }

    term_initialized = 1;
    atexit(restore_terminal);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGHUP, handle_signal);
    signal(SIGWINCH, handle_winch);

    struct winsize ws;
    int rows = 24, cols = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        rows = ws.ws_row;
        cols = ws.ws_col;
    }

    stdscr = calloc(1, sizeof(WINDOW));
    if (!stdscr) {
        return NULL;
    }
    stdscr->begy = 0;
    stdscr->begx = 0;
    stdscr->maxy = rows;
    stdscr->maxx = cols;
    stdscr->cury = 0;
    stdscr->curx = 0;
    stdscr->parent = NULL;
    stdscr->keypad_mode = 0;
    stdscr->attr = COLOR_PAIR(0);

    return stdscr;
}

int endwin(void) {
    restore_terminal();
    signal(SIGWINCH, SIG_DFL);
    extern void _vc_screen_free(void);
    _vc_screen_free();
    free(stdscr);
    stdscr = NULL;
    return 0;
}

