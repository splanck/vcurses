#ifndef CURSES_H
#define CURSES_H

#include "vcurses.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct window {
    int begy, begx; /* origin */
    int maxy, maxx; /* size */
    int cury, curx; /* cursor position */
    struct window *parent; /* parent for subwindows */
} WINDOW;

extern WINDOW *stdscr;

WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x);
int delwin(WINDOW *win);
WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
WINDOW *derwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
int wmove(WINDOW *win, int y, int x);
int waddstr(WINDOW *win, const char *str);

#ifdef __cplusplus
}
#endif

#endif /* CURSES_H */
