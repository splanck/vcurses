#include "curses.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

WINDOW *stdscr = NULL;


/* ----- Color and attribute handling ----- */

#define MAX_COLOR_PAIRS COLOR_PAIRS

typedef struct { short fg; short bg; } color_pair_t;
extern color_pair_t _vc_color_pairs[MAX_COLOR_PAIRS];
extern int _vc_colors_initialized;

static void apply_attr(int attr) {
    /* reset attributes */
    printf("\x1b[0m");

    if (attr & A_COLOR) {
        short pair = PAIR_NUMBER(attr);
        if (pair >= 0 && pair < MAX_COLOR_PAIRS && _vc_colors_initialized) {
            short fg = _vc_color_pairs[pair].fg;
            short bg = _vc_color_pairs[pair].bg;
            printf("\x1b[%d;%dm", 30 + fg, 40 + bg);
        }
    } else if (_vc_colors_initialized) {
        /* default colors */
        printf("\x1b[%d;%dm", 30 + _vc_color_pairs[0].fg, 40 + _vc_color_pairs[0].bg);
    }

    if (attr & A_BOLD)
        printf("\x1b[1m");
    if (attr & A_UNDERLINE)
        printf("\x1b[4m");
    if (attr & A_REVERSE)
        printf("\x1b[7m");
    if (attr & A_BLINK)
        printf("\x1b[5m");
    if (attr & A_DIM)
        printf("\x1b[2m");
    if (attr & A_STANDOUT)
        printf("\x1b[7m");
}

void _vcurses_apply_attr(int attr) {
    apply_attr(attr);
}

int wattron(WINDOW *win, int attrs) {
    if (!win)
        return -1;
    if (attrs & A_COLOR)
        win->attr &= ~A_COLOR;
    win->attr |= attrs;
    return 0;
}

int wattroff(WINDOW *win, int attrs) {
    if (!win)
        return -1;
    if (attrs & A_COLOR)
        win->attr &= ~A_COLOR;
    win->attr &= ~attrs;
    return 0;
}

int wattrset(WINDOW *win, int attrs) {
    if (!win)
        return -1;
    win->attr = attrs;
    return 0;
}

int attron(int attrs) { return wattron(stdscr, attrs); }
int attroff(int attrs) { return wattroff(stdscr, attrs); }
int attrset(int attrs) { return wattrset(stdscr, attrs); }

int wcolor_set(WINDOW *win, short pair, void *opts) {
    (void)opts;
    if (!win)
        return -1;
    win->attr &= ~A_COLOR;
    win->attr |= COLOR_PAIR(pair);
    return 0;
}

int color_set(short pair, void *opts) { return wcolor_set(stdscr, pair, opts); }

int move(int y, int x) {
    return wmove(stdscr, y, x);
}

int addstr(const char *str) {
    return waddstr(stdscr, str);
}

int addch(char ch) {
    return waddch(stdscr, ch);
}

int mvaddch(int y, int x, char ch) {
    return mvwaddch(stdscr, y, x, ch);
}

int mvaddstr(int y, int x, const char *str) {
    return mvwaddstr(stdscr, y, x, str);
}

int printw(const char *fmt, ...) {
    va_list ap, ap_copy;
    va_start(ap, fmt);
    va_copy(ap_copy, ap);
    int len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);
    if (len < 0) {
        va_end(ap);
        return -1;
    }

    char *buf = malloc((size_t)len + 1);
    if (!buf) {
        va_end(ap);
        return -1;
    }

    vsnprintf(buf, (size_t)len + 1, fmt, ap);
    va_end(ap);
    int r = waddstr(stdscr, buf);
    free(buf);
    return r;
}

int mvprintw(int y, int x, const char *fmt, ...) {
    if (move(y, x) == -1)
        return -1;

    va_list ap, ap_copy;
    va_start(ap, fmt);
    va_copy(ap_copy, ap);
    int len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);
    if (len < 0) {
        va_end(ap);
        return -1;
    }

    char *buf = malloc((size_t)len + 1);
    if (!buf) {
        va_end(ap);
        return -1;
    }

    vsnprintf(buf, (size_t)len + 1, fmt, ap);
    va_end(ap);
    int r = waddstr(stdscr, buf);
    free(buf);
    return r;
}

static int _cursor_state = 1;

int curs_set(int visibility) {
    int prev = _cursor_state;
    if (visibility == 0) {
        fputs("\x1b[?25l", stdout);
        _cursor_state = 0;
    } else if (visibility == 1) {
        fputs("\x1b[?25h", stdout);
        _cursor_state = 1;
    } else {
        return -1;
    }
    fflush(stdout);
    return prev;
}

int beep(void) {
    fputc('\a', stdout);
    return fflush(stdout);
}

int flash(void) {
    fputs("\x1b[?5h\x1b[?5l", stdout);
    return fflush(stdout);
}

int erase(void) {
    return werase(stdscr);
}
