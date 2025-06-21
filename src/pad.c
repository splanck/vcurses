#include "curses.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <wchar.h>

/* functions from resize.c */
extern void _vc_register_window(WINDOW *win);
extern void _vc_screen_puts(int y, int x, const char *str, int attr);

static WINDOW *pad_root(WINDOW *pad) {
    WINDOW *r = pad;
    while (r->parent && r->parent->is_pad)
        r = r->parent;
    return r;
}

WINDOW *newpad(int nlines, int ncols) {
    WINDOW *win = calloc(1, sizeof(WINDOW));
    if (!win)
        return NULL;
    win->begy = 0;
    win->begx = 0;
    win->maxy = nlines;
    win->maxx = ncols;
    win->cury = 0;
    win->curx = 0;
    win->parent = NULL;
    win->keypad_mode = 0;
    win->scroll = 0;
    win->top_margin = 0;
    win->bottom_margin = nlines ? nlines - 1 : 0;
    win->clearok = 0;
    win->delay = -1;
    win->meta_mode = 0;
    win->attr = COLOR_PAIR(0);
    win->bkgd = COLOR_PAIR(0);
    win->bkgd = COLOR_PAIR(0);
    win->is_pad = 1;
    win->pad_y = 0;
    win->pad_x = 0;
    win->pad_buf = malloc(sizeof(vcchar_t *) * nlines);
    win->pad_attr = malloc(sizeof(int *) * nlines);
    win->dirty = calloc(nlines, sizeof(unsigned char));
    if (!win->pad_buf || !win->pad_attr || !win->dirty) {
        free(win->pad_buf);
        free(win->pad_attr);
        free(win->dirty);
        free(win);
        return NULL;
    }
    for (int r = 0; r < nlines; ++r) {
        win->pad_buf[r] = malloc(sizeof(vcchar_t) * ncols);
        win->pad_attr[r] = malloc(sizeof(int) * ncols);
        if (!win->pad_buf[r] || !win->pad_attr[r]) {
            for (int i = 0; i <= r; ++i) {
                if (i < r) {
                    free(win->pad_buf[i]);
                    free(win->pad_attr[i]);
                }
            }
            free(win->pad_buf);
            free(win->pad_attr);
            free(win->dirty);
            free(win);
            return NULL;
        }
        for (int c = 0; c < ncols; ++c)
            win->pad_buf[r][c] = (vcchar_t)' ';
        for (int c = 0; c < ncols; ++c)
            win->pad_attr[r][c] = win->bkgd;
    }
    memset(win->dirty, 1, nlines);
    _vc_register_window(win);
    return win;
}

WINDOW *subpad(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x) {
    if (!orig || !orig->is_pad)
        return NULL;
    WINDOW *win = calloc(1, sizeof(WINDOW));
    if (!win)
        return NULL;
    win->begy = 0;
    win->begx = 0;
    win->maxy = nlines;
    win->maxx = ncols;
    win->cury = 0;
    win->curx = 0;
    win->parent = orig;
    win->keypad_mode = 0;
    win->scroll = 0;
    win->top_margin = 0;
    win->bottom_margin = nlines ? nlines - 1 : 0;
    win->clearok = 0;
    win->delay = -1;
    win->meta_mode = 0;
    win->attr = COLOR_PAIR(0);
    win->is_pad = 1;
    win->pad_y = orig->pad_y + begin_y;
    win->pad_x = orig->pad_x + begin_x;
    win->pad_buf = orig->pad_buf;
    win->pad_attr = orig->pad_attr;
    win->dirty = calloc(nlines, sizeof(unsigned char));
    if (!win->dirty) {
        free(win);
        return NULL;
    }
    memset(win->dirty, 1, nlines);
    _vc_register_window(win);
    return win;
}

int prefresh(WINDOW *pad, int pminrow, int pmincol,
             int sminrow, int smincol, int smaxrow, int smaxcol) {
    if (!pad || !pad->is_pad)
        return -1;
    WINDOW *root = pad_root(pad);
    int rows = smaxrow - sminrow + 1;
    int cols = smaxcol - smincol + 1;
    for (int r = 0; r < rows; ++r) {
        if (pminrow + r >= pad->maxy)
            break;
        for (int c = 0; c < cols; ++c) {
            if (pmincol + c >= pad->maxx)
                break;
            int rr = pad->pad_y + pminrow + r;
            int cc = pad->pad_x + pmincol + c;
            vcchar_t ch = root->pad_buf[rr][cc];
            int attr = root->pad_attr[rr][cc];
#ifdef VCURSES_WIDE
            char buf[MB_LEN_MAX + 1];
            mbstate_t st = {0};
            size_t n = wcrtomb(buf, ch, &st);
            if (n == (size_t)-1) {
                buf[0] = '?';
                n = 1;
            }
            buf[n] = '\0';
#else
            char buf[2] = { ch, 0 };
#endif
            _vc_screen_puts(sminrow + r, smincol + c, buf, attr);
        }
    }
    return 0;
}
