#include "curses.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* functions from resize.c */
extern void _vc_register_window(WINDOW *win);
extern void _vc_unregister_window(WINDOW *win);
static int vwprintw_internal(WINDOW *win, const char *fmt, va_list ap);

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
    win->is_pad = 0;
    win->pad_y = 0;
    win->pad_x = 0;
    win->pad_buf = NULL;
    win->pad_attr = NULL;
    _vc_register_window(win);
    return win;
}

int delwin(WINDOW *win) {
    if (!win || win == stdscr) {
        return -1;
    }
    _vc_unregister_window(win);
    if (win->is_pad && !win->parent) {
        if (win->pad_buf) {
            for (int r = 0; r < win->maxy; ++r)
                free(win->pad_buf[r]);
            free(win->pad_buf);
        }
        if (win->pad_attr) {
            for (int r = 0; r < win->maxy; ++r)
                free(win->pad_attr[r]);
            free(win->pad_attr);
        }
    }
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

static WINDOW *pad_root(WINDOW *pad) {
    WINDOW *r = pad;
    while (r->parent && r->parent->is_pad)
        r = r->parent;
    return r;
}

int mvwin(WINDOW *win, int y, int x) {
    if (!win)
        return -1;

    if (y < 0)
        y = 0;
    if (x < 0)
        x = 0;
    if (y + win->maxy > LINES)
        y = (LINES > win->maxy) ? LINES - win->maxy : 0;
    if (x + win->maxx > COLS)
        x = (COLS > win->maxx) ? COLS - win->maxx : 0;

    win->begy = y;
    win->begx = x;
    if (win->cury >= win->maxy)
        win->cury = win->maxy ? win->maxy - 1 : 0;
    if (win->curx >= win->maxx)
        win->curx = win->maxx ? win->maxx - 1 : 0;
    return 0;
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
    if (win->is_pad) {
        WINDOW *root = pad_root(win);
        int row = win->pad_y + win->cury;
        int col = win->pad_x + win->curx;
        for (const char *p = str; *p && row < root->maxy && col < root->maxx; ++p) {
            root->pad_buf[row][col] = *p;
            root->pad_attr[row][col] = win->attr;
            col++;
        }
        win->curx += strlen(str);
    } else {
        int row = win->begy + win->cury;
        int col = win->begx + win->curx;
        _vc_screen_puts(row, col, str, win->attr);
        win->curx += strlen(str);
    }
    return 0;
}

int waddch(WINDOW *win, char ch) {
    char buf[2];
    buf[0] = ch;
    buf[1] = '\0';
    return waddstr(win, buf);
}

int mvwaddch(WINDOW *win, int y, int x, char ch) {
    if (wmove(win, y, x) == -1)
        return -1;
    return waddch(win, ch);
}

int mvwaddstr(WINDOW *win, int y, int x, const char *str) {
    if (wmove(win, y, x) == -1)
        return -1;
    return waddstr(win, str);
}

int mvwprintw(WINDOW *win, int y, int x, const char *fmt, ...) {
    if (wmove(win, y, x) == -1)
        return -1;
    va_list ap;
    va_start(ap, fmt);
    int r = vwprintw_internal(win, fmt, ap);
    va_end(ap);
    return r;
}

static int vwprintw_internal(WINDOW *win, const char *fmt, va_list ap) {
    if (!win || !fmt)
        return -1;

    va_list ap_copy;
    va_copy(ap_copy, ap);
    int len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);
    if (len < 0)
        return -1;

    char *buf = malloc((size_t)len + 1);
    if (!buf)
        return -1;

    vsnprintf(buf, (size_t)len + 1, fmt, ap);
    int r = waddstr(win, buf);
    free(buf);
    return r;
}

int wprintw(WINDOW *win, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vwprintw_internal(win, fmt, ap);
    va_end(ap);
    return r;
}

int scrollok(WINDOW *win, bool bf) {
    if (!win)
        return -1;
    win->scroll = bf ? 1 : 0;
    return 0;
}

/*
 * Scroll the specified window by the given number of lines. Positive values
 * scroll upward, while negative values scroll downward.
 */
int wscrl(WINDOW *win, int lines) {
    if (!win)
        return -1;
    if (lines == 0)
        return 0;

    int count = lines;
    if (lines > win->maxy)
        count = win->maxy;
    else if (lines < -win->maxy)
        count = -win->maxy;

    _vc_screen_scroll_region(win->begy, win->begx, win->maxy, win->maxx,
                             count, win->attr);
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

    for (int x = 0; x < width; ++x) {
        if (x == 0)
            buf[0] = tl;
        else if (x == width - 1)
            buf[0] = tr;
        else
            buf[0] = ts;
        if (win->is_pad) {
            WINDOW *root = pad_root(win);
            int rr = win->pad_y;
            int cc = win->pad_x + x;
            root->pad_buf[rr][cc] = buf[0];
            root->pad_attr[rr][cc] = win->attr;
        } else {
            _vc_screen_puts(win->begy, win->begx + x, buf, win->attr);
        }

        if (height > 1) {
            if (x == 0)
                buf[0] = bl;
            else if (x == width - 1)
                buf[0] = br;
            else
                buf[0] = bs;
            if (win->is_pad) {
                WINDOW *root = pad_root(win);
                int rr = win->pad_y + height - 1;
                int cc = win->pad_x + x;
                root->pad_buf[rr][cc] = buf[0];
                root->pad_attr[rr][cc] = win->attr;
            } else {
                _vc_screen_puts(win->begy + height - 1, win->begx + x,
                                buf, win->attr);
            }
        }
    }

    for (int y = 1; y < height - 1; ++y) {
        buf[0] = ls;
        if (win->is_pad) {
            WINDOW *root = pad_root(win);
            int rr = win->pad_y + y;
            int cc = win->pad_x;
            root->pad_buf[rr][cc] = buf[0];
            root->pad_attr[rr][cc] = win->attr;
        } else {
            _vc_screen_puts(win->begy + y, win->begx, buf, win->attr);
        }
        if (width > 1) {
            buf[0] = rs;
            if (win->is_pad) {
                WINDOW *root = pad_root(win);
                int rr = win->pad_y + y;
                int cc = win->pad_x + width - 1;
                root->pad_buf[rr][cc] = buf[0];
                root->pad_attr[rr][cc] = win->attr;
            } else {
                _vc_screen_puts(win->begy + y, win->begx + width - 1,
                                buf, win->attr);
            }
        }
    }

    return 0;
}

int box(WINDOW *win, char verch, char horch) {
    return wborder(win, verch, verch, horch, horch,
                   0, 0, 0, 0);
}

int whline(WINDOW *win, char ch, int n) {
    if (!win || n <= 0)
        return -1;
    if (!ch)
        ch = '-';

    char buf[2] = { ch, 0 };
    int drawn = 0;
    for (int i = 0; i < n && win->curx + i < win->maxx; ++i) {
        if (win->is_pad) {
            WINDOW *root = pad_root(win);
            int rr = win->pad_y + win->cury;
            int cc = win->pad_x + win->curx + i;
            if (rr >= root->maxy || cc >= root->maxx)
                break;
            root->pad_buf[rr][cc] = ch;
            root->pad_attr[rr][cc] = win->attr;
        } else {
            _vc_screen_puts(win->begy + win->cury,
                            win->begx + win->curx + i,
                            buf, win->attr);
        }
        drawn++;
    }
    win->curx += drawn;
    return 0;
}

int hline(char ch, int n) {
    return whline(stdscr, ch, n);
}

int wvline(WINDOW *win, char ch, int n) {
    if (!win || n <= 0)
        return -1;
    if (!ch)
        ch = '|';

    char buf[2] = { ch, 0 };
    int drawn = 0;
    for (int i = 0; i < n && win->cury + i < win->maxy; ++i) {
        if (win->is_pad) {
            WINDOW *root = pad_root(win);
            int rr = win->pad_y + win->cury + i;
            int cc = win->pad_x + win->curx;
            if (rr >= root->maxy || cc >= root->maxx)
                break;
            root->pad_buf[rr][cc] = ch;
            root->pad_attr[rr][cc] = win->attr;
        } else {
            _vc_screen_puts(win->begy + win->cury + i,
                            win->begx + win->curx,
                            buf, win->attr);
        }
        drawn++;
    }
    win->cury += drawn;
    return 0;
}

int vline(char ch, int n) {
    return wvline(stdscr, ch, n);
}

extern void _vc_screen_refresh_region(int top, int left, int height, int width);

int wrefresh(WINDOW *win) {
    if (!win)
        return -1;
    _vc_screen_refresh_region(win->begy, win->begx, win->maxy, win->maxx);
    printf("\x1b[%d;%dH", win->begy + win->cury + 1, win->begx + win->curx + 1);
    return fflush(stdout);
}

/* erase a window's contents without forcing a terminal clear */
int werase(WINDOW *win) {
    if (!win)
        return -1;

    if (win->is_pad) {
        WINDOW *root = pad_root(win);
        for (int r = 0; r < win->maxy; ++r) {
            int rr = win->pad_y + r;
            if (rr >= root->maxy)
                break;
            for (int c = 0; c < win->maxx && win->pad_x + c < root->maxx; ++c) {
                int cc = win->pad_x + c;
                root->pad_buf[rr][cc] = ' ';
                root->pad_attr[rr][cc] = win->attr;
            }
        }
    } else {
        char *spaces = malloc(win->maxx + 1);
        if (!spaces)
            return -1;
        memset(spaces, ' ', win->maxx);
        spaces[win->maxx] = '\0';
        for (int r = 0; r < win->maxy; ++r)
            _vc_screen_puts(win->begy + r, win->begx, spaces, win->attr);
        free(spaces);
    }

    win->cury = 0;
    win->curx = 0;
    return 0;
}

/* clear the entire window by writing spaces into its backing buffer */
int wclear(WINDOW *win) {
    if (!win)
        return -1;

    if (win->is_pad) {
        WINDOW *root = pad_root(win);
        for (int r = 0; r < win->maxy; ++r) {
            int rr = win->pad_y + r;
            if (rr >= root->maxy)
                break;
            for (int c = 0; c < win->maxx && win->pad_x + c < root->maxx; ++c) {
                int cc = win->pad_x + c;
                root->pad_buf[rr][cc] = ' ';
                root->pad_attr[rr][cc] = win->attr;
            }
        }
    } else {
        char *spaces = malloc(win->maxx + 1);
        if (!spaces)
            return -1;
        memset(spaces, ' ', win->maxx);
        spaces[win->maxx] = '\0';
        for (int r = 0; r < win->maxy; ++r)
            _vc_screen_puts(win->begy + r, win->begx, spaces, win->attr);
        free(spaces);
    }

    win->cury = 0;
    win->curx = 0;
    return 0;
}

/* clear from the cursor to end of line */
int wclrtoeol(WINDOW *win) {
    if (!win)
        return -1;

    int len = win->maxx - win->curx;
    if (len <= 0)
        return 0;

    if (win->is_pad) {
        WINDOW *root = pad_root(win);
        int rr = win->pad_y + win->cury;
        int cc = win->pad_x + win->curx;
        if (rr >= root->maxy || cc >= root->maxx)
            return -1;
        for (int c = 0; c < len && cc + c < root->maxx; ++c) {
            root->pad_buf[rr][cc + c] = ' ';
            root->pad_attr[rr][cc + c] = win->attr;
        }
    } else {
        char *spaces = malloc(len + 1);
        if (!spaces)
            return -1;
        memset(spaces, ' ', len);
        spaces[len] = '\0';
        _vc_screen_puts(win->begy + win->cury, win->begx + win->curx,
                        spaces, win->attr);
        free(spaces);
    }
    return 0;
}

/* clear from cursor to bottom of window */
int wclrtobot(WINDOW *win) {
    if (!win)
        return -1;

    for (int r = win->cury; r < win->maxy; ++r) {
        int start = (r == win->cury) ? win->curx : 0;
        int len = win->maxx - start;
        if (len <= 0)
            continue;
        if (win->is_pad) {
            WINDOW *root = pad_root(win);
            int rr = win->pad_y + r;
            int cc = win->pad_x + start;
            if (rr >= root->maxy || cc >= root->maxx)
                break;
            for (int c = 0; c < len && cc + c < root->maxx; ++c) {
                root->pad_buf[rr][cc + c] = ' ';
                root->pad_attr[rr][cc + c] = win->attr;
            }
        } else {
            char *spaces = malloc(len + 1);
            if (!spaces)
                return -1;
            memset(spaces, ' ', len);
            spaces[len] = '\0';
            _vc_screen_puts(win->begy + r, win->begx + start,
                            spaces, win->attr);
            free(spaces);
        }
    }
    return 0;
}

int wresize(WINDOW *win, int nlines, int ncols) {
    if (!win || nlines < 0 || ncols < 0)
        return -1;

    if (win->is_pad) {
        WINDOW *root = pad_root(win);
        if (!win->parent) {
            char **nbuf = malloc(sizeof(char *) * nlines);
            int **nattr = malloc(sizeof(int *) * nlines);
            if (!nbuf || !nattr) {
                free(nbuf);
                free(nattr);
                return -1;
            }
            for (int r = 0; r < nlines; ++r) {
                nbuf[r] = malloc(ncols);
                nattr[r] = malloc(sizeof(int) * ncols);
                if (!nbuf[r] || !nattr[r]) {
                    for (int i = 0; i <= r; ++i) {
                        if (i < r) {
                            free(nbuf[i]);
                            free(nattr[i]);
                        }
                    }
                    free(nbuf);
                    free(nattr);
                    return -1;
                }
            }

            int copy_rows = (nlines < root->maxy) ? nlines : root->maxy;
            int copy_cols = (ncols < root->maxx) ? ncols : root->maxx;
            for (int r = 0; r < nlines; ++r) {
                for (int c = 0; c < ncols; ++c) {
                    if (r < copy_rows && c < copy_cols) {
                        nbuf[r][c] = root->pad_buf[r][c];
                        nattr[r][c] = root->pad_attr[r][c];
                    } else {
                        nbuf[r][c] = ' ';
                        nattr[r][c] = win->attr;
                    }
                }
            }

            for (int r = 0; r < root->maxy; ++r) {
                free(root->pad_buf[r]);
                free(root->pad_attr[r]);
            }
            free(root->pad_buf);
            free(root->pad_attr);
            root->pad_buf = nbuf;
            root->pad_attr = nattr;
            root->maxy = nlines;
            root->maxx = ncols;
        } else {
            if (win->pad_y + nlines > root->maxy)
                nlines = root->maxy - win->pad_y;
            if (win->pad_x + ncols > root->maxx)
                ncols = root->maxx - win->pad_x;
        }
    } else {
        if (win->begy + nlines > LINES)
            nlines = LINES - win->begy;
        if (win->begx + ncols > COLS)
            ncols = COLS - win->begx;
    }

    win->maxy = nlines;
    win->maxx = ncols;
    if (win->cury >= win->maxy)
        win->cury = win->maxy ? win->maxy - 1 : 0;
    if (win->curx >= win->maxx)
        win->curx = win->maxx ? win->maxx - 1 : 0;
    return 0;
}
