#include "curses.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

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
