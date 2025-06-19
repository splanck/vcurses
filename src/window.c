#include "curses.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* functions from resize.c */
extern void _vc_register_window(WINDOW *win);
extern void _vc_unregister_window(WINDOW *win);

WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x) {
    WINDOW *win = calloc(1, sizeof(WINDOW));
    if (!win) {
        return NULL;
    }
    win->begy = begin_y;
    win->begx = begin_x;
    win->maxy = nlines;
    win->maxx = ncols;
    win->cury = 0;
    win->curx = 0;
    win->parent = NULL;
    win->keypad_mode = 0;
    win->scroll = 0;
    win->delay = -1;
    win->attr = COLOR_PAIR(0);
    _vc_register_window(win);
    return win;
}

int delwin(WINDOW *win) {
    if (!win || win == stdscr) {
        return -1;
    }
    _vc_unregister_window(win);
    free(win);
    return 0;
}

WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x) {
    if (!orig) {
        return NULL;
    }
    WINDOW *win = newwin(nlines, ncols, begin_y, begin_x);
    if (win) {
        win->parent = orig;
    }
    return win;
}

WINDOW *derwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x) {
    if (!orig) {
        return NULL;
    }
    return subwin(orig, nlines, ncols, orig->begy + begin_y, orig->begx + begin_x);
}

int wmove(WINDOW *win, int y, int x) {
    if (!win) {
        return -1;
    }
    if (y < 0 || y >= win->maxy || x < 0 || x >= win->maxx) {
        return -1;
    }
    win->cury = y;
    win->curx = x;
    return 0;
}

extern void _vc_screen_puts(int y, int x, const char *str, int attr);
extern void _vc_screen_scroll_region(int top, int left, int height, int width,
                                     int lines, int attr);

int waddstr(WINDOW *win, const char *str) {
    if (!win || !str) {
        return -1;
    }
    int row = win->begy + win->cury;
    int col = win->begx + win->curx;
    _vc_screen_puts(row, col, str, win->attr);
    win->curx += strlen(str);
    return 0;
}

int waddch(WINDOW *win, char ch) {
    char buf[2];
    buf[0] = ch;
    buf[1] = '\0';
    return waddstr(win, buf);
}

int scrollok(WINDOW *win, bool bf) {
    if (!win)
        return -1;
    win->scroll = bf ? 1 : 0;
    return 0;
}

int wscrl(WINDOW *win, int lines) {
    if (!win)
        return -1;
    if (lines == 0)
        return 0;
    if (lines > 0) {
        _vc_screen_scroll_region(win->begy, win->begx, win->maxy, win->maxx,
                                 lines, win->attr);
    } else {
        /* downward scrolling not implemented */
        return -1;
    }
    return 0;
}

int wborder(WINDOW *win,
            char ls, char rs, char ts, char bs,
            char tl, char tr, char bl, char br) {
    if (!win)
        return -1;

    /* default ASCII characters */
    if (!ls) ls = '|';
    if (!rs) rs = '|';
    if (!ts) ts = '-';
    if (!bs) bs = '-';
    if (!tl) tl = '+';
    if (!tr) tr = '+';
    if (!bl) bl = '+';
    if (!br) br = '+';

    int height = win->maxy;
    int width = win->maxx;

    char buf[2] = {0, 0};

    /* top and bottom */
    for (int x = 0; x < width; ++x) {
        if (x == 0)
            buf[0] = tl;
        else if (x == width - 1)
            buf[0] = tr;
        else
            buf[0] = ts;
        _vc_screen_puts(win->begy, win->begx + x, buf, win->attr);

        if (height > 1) {
            if (x == 0)
                buf[0] = bl;
            else if (x == width - 1)
                buf[0] = br;
            else
                buf[0] = bs;
            _vc_screen_puts(win->begy + height - 1, win->begx + x,
                            buf, win->attr);
        }
    }

    /* left and right sides */
    for (int y = 1; y < height - 1; ++y) {
        buf[0] = ls;
        _vc_screen_puts(win->begy + y, win->begx, buf, win->attr);
        if (width > 1) {
            buf[0] = rs;
            _vc_screen_puts(win->begy + y, win->begx + width - 1,
                            buf, win->attr);
        }
    }

    return 0;
}

int box(WINDOW *win, char verch, char horch) {
    return wborder(win, verch, verch, horch, horch,
                   0, 0, 0, 0);
}
