#include "curses.h"
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

/* resize helpers */
extern void _vc_resize_init(void);
extern void _vc_resize_shutdown(void);

struct termios orig_termios;
static struct termios saved_termios;
static int term_initialized = 0;

static void restore_terminal(void) {
    if (term_initialized) {
        tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios);
        term_initialized = 0;
    }
}

static void handle_signal(int sig) {
    restore_terminal();
    signal(sig, SIG_DFL);
    raise(sig);
}


WINDOW *initscr(void) {
    if (tcgetattr(STDIN_FILENO, &saved_termios) == -1) {
        return NULL;
    }

    orig_termios = saved_termios;
    orig_termios.c_lflag &= ~(ECHO | ICANON);
    orig_termios.c_cc[VMIN] = 1;
    orig_termios.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios) == -1) {
        return NULL;
    }

    term_initialized = 1;
    atexit(restore_terminal);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGHUP, handle_signal);
    _vc_resize_init();

    int rows = LINES;
    int cols = COLS;

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
    stdscr->scroll = 0;
    stdscr->top_margin = 0;
    stdscr->bottom_margin = rows ? rows - 1 : 0;
    stdscr->delay = -1;
    stdscr->attr = COLOR_PAIR(0);
    stdscr->bkgd = COLOR_PAIR(0);

    return stdscr;
}

int endwin(void) {
    restore_terminal();
    _vc_resize_shutdown();
    extern void _vc_screen_free(void);
    _vc_screen_free();
    free(stdscr);
    stdscr = NULL;
    return 0;
}

