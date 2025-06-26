#include "curses.h"
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

/* resize helpers */
extern void _vc_resize_init(void);
extern void _vc_resize_shutdown(void);

SCREEN *_vc_current_screen = NULL;

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


static int ensure_term_initialized(void) {
    if (term_initialized)
        return 0;
    if (tcgetattr(STDIN_FILENO, &saved_termios) == -1)
        return -1;
    orig_termios = saved_termios;
    orig_termios.c_lflag &= ~(ECHO | ICANON);
    orig_termios.c_cc[VMIN] = 1;
    orig_termios.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios) == -1)
        return -1;
    term_initialized = 1;
    atexit(restore_terminal);
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGHUP, handle_signal);
    _vc_resize_init();
    return 0;
}

static SCREEN *alloc_screen(void) {
    SCREEN *scr = calloc(1, sizeof(SCREEN));
    if (!scr)
        return NULL;
    int rows = LINES;
    int cols = COLS;
    scr->stdscr = calloc(1, sizeof(WINDOW));
    if (!scr->stdscr) {
        free(scr);
        return NULL;
    }
    scr->stdscr->begy = 0;
    scr->stdscr->begx = 0;
    scr->stdscr->maxy = rows;
    scr->stdscr->maxx = cols;
    scr->stdscr->cury = 0;
    scr->stdscr->curx = 0;
    scr->stdscr->parent = NULL;
    scr->stdscr->keypad_mode = 0;
    scr->stdscr->scroll = 0;
    scr->stdscr->top_margin = 0;
    scr->stdscr->bottom_margin = rows ? rows - 1 : 0;
    scr->stdscr->delay = -1;
    scr->stdscr->meta_mode = 0;
    scr->stdscr->attr = COLOR_PAIR(0);
    scr->stdscr->bkgd = COLOR_PAIR(0);
    scr->lines = rows;
    scr->cols = cols;
    scr->cursor_y = scr->cursor_x = -1;
    return scr;
}

SCREEN *newterm(const char *type, FILE *outf, FILE *inf) {
    (void)type; (void)outf; (void)inf;
    vc_init();
    if (ensure_term_initialized() == -1)
        return NULL;
    return alloc_screen();
}

SCREEN *set_term(SCREEN *scr) {
    SCREEN *old = _vc_current_screen;
    if (scr) {
        _vc_current_screen = scr;
        stdscr = scr->stdscr;
        LINES = scr->lines;
        COLS = scr->cols;
    }
    return old;
}

WINDOW *initscr(void) {
    SCREEN *scr = newterm(NULL, stdout, stdin);
    if (!scr)
        return NULL;
    set_term(scr);
    return stdscr;
}

int endwin(void) {
    restore_terminal();
    _vc_resize_shutdown();
    extern void _vc_screen_free(void);
    _vc_screen_free();
    if (_vc_current_screen) {
        free(_vc_current_screen->stdscr);
        _vc_current_screen->stdscr = NULL;
    }
    stdscr = NULL;
    return 0;
}

