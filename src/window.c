#include "curses.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <wchar.h>

/* functions from resize.c */
extern void _vc_register_window(WINDOW *win);
extern void _vc_unregister_window(WINDOW *win);
static int vwprintw_internal(WINDOW *win, const char *fmt, va_list ap);

static void mark_dirty(WINDOW *win, int start, int count)
{
    if (!win || !win->dirty)
        return;
    if (start < 0)
        start = 0;
    if (count < 0)
        count = 0;
    if (start >= win->maxy)
        return;
    if (start + count > win->maxy)
        count = win->maxy - start;
    memset(&win->dirty[start], 1, (size_t)count);
}

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
    win->top_margin = 0;
    win->bottom_margin = nlines ? nlines - 1 : 0;
    win->clearok = 0;
    win->delay = -1;
    win->attr = COLOR_PAIR(0);
    win->bkgd = COLOR_PAIR(0);
    win->is_pad = 0;
    win->pad_y = 0;
    win->pad_x = 0;
    win->pad_buf = NULL;
    win->pad_attr = NULL;
    win->dirty = calloc(nlines, sizeof(unsigned char));
    if (!win->dirty) {
        free(win);
        return NULL;
    }
    memset(win->dirty, 1, nlines);
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
    free(win->dirty);
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
extern int _vc_screen_get_cell(int y, int x, char *ch, int *attr);

int waddstr(WINDOW *win, const char *str) {
    if (!win || !str) {
        return -1;
    }
    if (win->is_pad) {
        WINDOW *root = pad_root(win);
        int row = win->pad_y + win->cury;
        int col = win->pad_x + win->curx;
        for (const char *p = str; *p && row < root->maxy && col < root->maxx; ++p) {
            root->pad_buf[row][col] = (vcchar_t)*p;
            root->pad_attr[row][col] = win->attr;
            col++;
        }
        win->curx += strlen(str);
        mark_dirty(win, win->cury, 1);
    } else {
        int row = win->begy + win->cury;
        int col = win->begx + win->curx;
        _vc_screen_puts(row, col, str, win->attr);
        win->curx += strlen(str);
        mark_dirty(win, win->cury, 1);
    }
    return 0;
}

int waddch(WINDOW *win, char ch) {
    char buf[2];
    buf[0] = ch;
    buf[1] = '\0';
    return waddstr(win, buf);
}

int wadd_wch(WINDOW *win, wchar_t wch) {
    if (!win)
        return -1;
#ifdef VCURSES_WIDE
    if (win->is_pad) {
        WINDOW *root = pad_root(win);
        int row = win->pad_y + win->cury;
        int col = win->pad_x + win->curx;
        if (row >= root->maxy || col >= root->maxx)
            return -1;
        root->pad_buf[row][col] = wch;
        root->pad_attr[row][col] = win->attr;
        win->curx++;
        mark_dirty(win, win->cury, 1);
        return 0;
    }
#endif
    char mb[MB_LEN_MAX + 1];
    mbstate_t st = {0};
    size_t n = wcrtomb(mb, wch, &st);
    if (n == (size_t)-1)
        return -1;
    mb[n] = '\0';
    return waddstr(win, mb);
}

int add_wch(wchar_t wch) {
    return wadd_wch(stdscr, wch);
}

int mvwaddch(WINDOW *win, int y, int x, char ch) {
    if (wmove(win, y, x) == -1)
        return -1;
    return waddch(win, ch);
}

int mvwadd_wch(WINDOW *win, int y, int x, wchar_t wch) {
    if (wmove(win, y, x) == -1)
        return -1;
    return wadd_wch(win, wch);
}

int mvadd_wch(int y, int x, wchar_t wch) {
    return mvwadd_wch(stdscr, y, x, wch);
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

int clearok(WINDOW *win, bool bf) {
    if (!win)
        return -1;
    win->clearok = bf ? 1 : 0;
    return 0;
}

int wsetscrreg(WINDOW *win, int top, int bottom) {
    if (!win || top < 0 || bottom < top || bottom >= win->maxy)
        return -1;
    win->top_margin = top;
    win->bottom_margin = bottom;
    return 0;
}

int setscrreg(int top, int bottom) {
    return wsetscrreg(stdscr, top, bottom);
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

    int region_top = win->begy + win->top_margin;
    int height = win->bottom_margin - win->top_margin + 1;

    int count = lines;
    if (lines > height)
        count = height;
    else if (lines < -height)
        count = -height;

    _vc_screen_scroll_region(region_top, win->begx, height, win->maxx,
                             count, win->bkgd);
    mark_dirty(win, win->top_margin, height);
    return 0;
}

/* scroll a window by one line if scrolling is enabled */
int scroll(WINDOW *win) {
    if (!win || !win->scroll)
        return -1;
    return wscrl(win, 1);
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
            root->pad_buf[rr][cc] = (vcchar_t)buf[0];
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
                root->pad_buf[rr][cc] = (vcchar_t)buf[0];
                root->pad_attr[rr][cc] = win->bkgd;
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
            root->pad_buf[rr][cc] = (vcchar_t)buf[0];
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
                root->pad_buf[rr][cc] = (vcchar_t)buf[0];
                root->pad_attr[rr][cc] = win->bkgd;
            } else {
                _vc_screen_puts(win->begy + y, win->begx + width - 1,
                                buf, win->attr);
            }
        }
    }
    mark_dirty(win, 0, win->maxy);
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
            root->pad_buf[rr][cc] = (vcchar_t)ch;
            root->pad_attr[rr][cc] = win->attr;
        } else {
            _vc_screen_puts(win->begy + win->cury,
                            win->begx + win->curx + i,
                            buf, win->attr);
        }
        drawn++;
    }
    win->curx += drawn;
    mark_dirty(win, win->cury, 1);
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
            root->pad_buf[rr][cc] = (vcchar_t)ch;
            root->pad_attr[rr][cc] = win->attr;
        } else {
            _vc_screen_puts(win->begy + win->cury + i,
                            win->begx + win->curx,
                            buf, win->attr);
        }
        drawn++;
    }
    win->cury += drawn;
    mark_dirty(win, win->cury - drawn, drawn);
    return 0;
}

int vline(char ch, int n) {
    return wvline(stdscr, ch, n);
}

extern int wnoutrefresh(WINDOW *win);
extern int doupdate(void);

int wrefresh(WINDOW *win) {
    if (!win)
        return -1;
    if (wnoutrefresh(win) == -1)
        return -1;
    return doupdate();
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
                root->pad_buf[rr][cc] = (vcchar_t)' ';
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
            _vc_screen_puts(win->begy + r, win->begx, spaces, win->bkgd);
        free(spaces);
    }

    win->cury = 0;
    win->curx = 0;
    mark_dirty(win, 0, win->maxy);
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
                root->pad_buf[rr][cc] = (vcchar_t)' ';
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
            _vc_screen_puts(win->begy + r, win->begx, spaces, win->bkgd);
        free(spaces);
    }

    win->cury = 0;
    win->curx = 0;
    mark_dirty(win, 0, win->maxy);
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
            root->pad_buf[rr][cc + c] = (vcchar_t)' ';
            root->pad_attr[rr][cc + c] = win->bkgd;
        }
    } else {
        char *spaces = malloc(len + 1);
        if (!spaces)
            return -1;
        memset(spaces, ' ', len);
        spaces[len] = '\0';
        _vc_screen_puts(win->begy + win->cury, win->begx + win->curx,
                        spaces, win->bkgd);
        free(spaces);
    }
    mark_dirty(win, win->cury, 1);
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
                root->pad_buf[rr][cc + c] = (vcchar_t)' ';
                root->pad_attr[rr][cc + c] = win->bkgd;
            }
        } else {
            char *spaces = malloc(len + 1);
            if (!spaces)
                return -1;
            memset(spaces, ' ', len);
            spaces[len] = '\0';
            _vc_screen_puts(win->begy + r, win->begx + start,
                            spaces, win->bkgd);
            free(spaces);
        }
    }
    mark_dirty(win, win->cury, win->maxy - win->cury);
    return 0;
}

static int insert_char_screen(int y, int x, int max, char ch, int attr)
{
    for (int i = max - 1; i > 0; --i) {
        char prev = ' ';
        int prev_attr = attr;
        _vc_screen_get_cell(y, x + i - 1, &prev, &prev_attr);
        char buf[2] = { prev, 0 };
        _vc_screen_puts(y, x + i, buf, prev_attr);
    }
    char buf[2] = { ch, 0 };
    _vc_screen_puts(y, x, buf, attr);
    return 0;
}

static int delete_char_screen(int y, int x, int max, int attr)
{
    for (int i = 0; i < max - 1; ++i) {
        char next = ' ';
        int next_attr = attr;
        _vc_screen_get_cell(y, x + i + 1, &next, &next_attr);
        char buf[2] = { next, 0 };
        _vc_screen_puts(y, x + i, buf, next_attr);
    }
    char buf[2] = { ' ', 0 };
    _vc_screen_puts(y, x + max - 1, buf, attr);
    return 0;
}

int winsch(WINDOW *win, char ch)
{
    if (!win)
        return -1;
    int len = win->maxx - win->curx;
    if (len <= 0)
        return -1;

    if (win->is_pad) {
        WINDOW *root = pad_root(win);
        int rr = win->pad_y + win->cury;
        int cc = win->pad_x + win->curx;
        if (rr >= root->maxy || cc >= root->maxx)
            return -1;
        if (cc + len > root->maxx)
            len = root->maxx - cc;
        for (int i = len - 1; i > 0; --i) {
            root->pad_buf[rr][cc + i] = root->pad_buf[rr][cc + i - 1];
            root->pad_attr[rr][cc + i] = root->pad_attr[rr][cc + i - 1];
        }
        root->pad_buf[rr][cc] = (vcchar_t)ch;
        root->pad_attr[rr][cc] = win->attr;
    } else {
        int row = win->begy + win->cury;
        int col = win->begx + win->curx;
        if (col + len > COLS)
            len = COLS - col;
        insert_char_screen(row, col, len, ch, win->attr);
    }
    mark_dirty(win, win->cury, 1);
    return 0;
}

int mvwinsch(WINDOW *win, int y, int x, char ch)
{
    if (wmove(win, y, x) == -1)
        return -1;
    return winsch(win, ch);
}

int insch(char ch)
{
    return winsch(stdscr, ch);
}

int mvinsch(int y, int x, char ch)
{
    return mvwinsch(stdscr, y, x, ch);
}

int wdelch(WINDOW *win)
{
    if (!win)
        return -1;
    int len = win->maxx - win->curx;
    if (len <= 0)
        return -1;

    if (win->is_pad) {
        WINDOW *root = pad_root(win);
        int rr = win->pad_y + win->cury;
        int cc = win->pad_x + win->curx;
        if (rr >= root->maxy || cc >= root->maxx)
            return -1;
        if (cc + len > root->maxx)
            len = root->maxx - cc;
        for (int i = 0; i < len - 1; ++i) {
            root->pad_buf[rr][cc + i] = root->pad_buf[rr][cc + i + 1];
            root->pad_attr[rr][cc + i] = root->pad_attr[rr][cc + i + 1];
        }
        root->pad_buf[rr][cc + len - 1] = (vcchar_t)' ';
        root->pad_attr[rr][cc + len - 1] = win->attr;
    } else {
        int row = win->begy + win->cury;
        int col = win->begx + win->curx;
        if (col + len > COLS)
            len = COLS - col;
        delete_char_screen(row, col, len, win->attr);
    }
    mark_dirty(win, win->cury, 1);
    return 0;
}

int mvwdelch(WINDOW *win, int y, int x)
{
    if (wmove(win, y, x) == -1)
        return -1;
    return wdelch(win);
}

int delch(void)
{
    return wdelch(stdscr);
}

int mvdelch(int y, int x)
{
    return mvwdelch(stdscr, y, x);
}

int winsstr(WINDOW *win, const char *str)
{
    if (!win || !str)
        return -1;
    size_t len = strlen(str);
    for (size_t i = len; i-- > 0;) {
        if (winsch(win, str[i]) == -1)
            return -1;
    }
    return 0;
}

int mvwinsstr(WINDOW *win, int y, int x, const char *str)
{
    if (wmove(win, y, x) == -1)
        return -1;
    return winsstr(win, str);
}

int insstr(const char *str)
{
    return winsstr(stdscr, str);
}

int mvinsstr(int y, int x, const char *str)
{
    return mvwinsstr(stdscr, y, x, str);
}

int winsdelln(WINDOW *win, int n)
{
    if (!win || n == 0)
        return win ? 0 : -1;

    int height = win->maxy - win->cury;
    if (height <= 0)
        return 0;

    if (n > height)
        n = height;
    else if (n < -height)
        n = -height;

    if (win->is_pad) {
        WINDOW *root = pad_root(win);
        int start = win->pad_y + win->cury;
        int width = win->maxx;
        if (start >= root->maxy || win->pad_x >= root->maxx)
            return -1;
        if (start + height > root->maxy)
            height = root->maxy - start;
        if (win->pad_x + width > root->maxx)
            width = root->maxx - win->pad_x;
        if (n > 0) {
            for (int r = height - 1; r >= n; --r) {
                memmove(&root->pad_buf[start + r][win->pad_x],
                        &root->pad_buf[start + r - n][win->pad_x],
                        sizeof(vcchar_t) * (size_t)width);
                memmove(&root->pad_attr[start + r][win->pad_x],
                        &root->pad_attr[start + r - n][win->pad_x],
                        sizeof(int) * (size_t)width);
            }
            for (int r = 0; r < n; ++r) {
                for (int c = 0; c < width; ++c)
                    root->pad_buf[start + r][win->pad_x + c] = (vcchar_t)' ';
                for (int c = 0; c < width; ++c)
                    root->pad_attr[start + r][win->pad_x + c] = win->attr;
            }
        } else {
            n = -n;
            for (int r = 0; r < height - n; ++r) {
                memmove(&root->pad_buf[start + r][win->pad_x],
                        &root->pad_buf[start + r + n][win->pad_x],
                        sizeof(vcchar_t) * (size_t)width);
                memmove(&root->pad_attr[start + r][win->pad_x],
                        &root->pad_attr[start + r + n][win->pad_x],
                        sizeof(int) * (size_t)width);
            }
            for (int r = height - n; r < height; ++r) {
                for (int c = 0; c < width; ++c)
                    root->pad_buf[start + r][win->pad_x + c] = (vcchar_t)' ';
                for (int c = 0; c < width; ++c)
                    root->pad_attr[start + r][win->pad_x + c] = win->attr;
            }
        }
    } else {
        _vc_screen_scroll_region(win->begy + win->cury, win->begx,
                                 height, win->maxx, -n, win->attr);
    }

    mark_dirty(win, win->cury, height);
    return 0;
}

int insertln(void)
{
    return winsdelln(stdscr, 1);
}

int deleteln(void)
{
    return winsdelln(stdscr, -1);
}

int wresize(WINDOW *win, int nlines, int ncols) {
    if (!win || nlines < 0 || ncols < 0)
        return -1;

    if (win->is_pad) {
        WINDOW *root = pad_root(win);
        if (!win->parent) {
            vcchar_t **nbuf = malloc(sizeof(vcchar_t *) * nlines);
            int **nattr = malloc(sizeof(int *) * nlines);
            if (!nbuf || !nattr) {
                free(nbuf);
                free(nattr);
                return -1;
            }
            for (int r = 0; r < nlines; ++r) {
                nbuf[r] = malloc(sizeof(vcchar_t) * ncols);
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
                        nbuf[r][c] = (vcchar_t)' ';
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
    if (win->top_margin >= nlines)
        win->top_margin = 0;
    if (win->bottom_margin >= nlines)
        win->bottom_margin = nlines ? nlines - 1 : 0;
    if (win->top_margin > win->bottom_margin) {
        win->top_margin = 0;
        win->bottom_margin = nlines ? nlines - 1 : 0;
    }
    free(win->dirty);
    win->dirty = calloc(nlines, sizeof(unsigned char));
    if (!win->dirty)
        return -1;
    memset(win->dirty, 1, nlines);
    if (win->cury >= win->maxy)
        win->cury = win->maxy ? win->maxy - 1 : 0;
    if (win->curx >= win->maxx)
        win->curx = win->maxx ? win->maxx - 1 : 0;
    return 0;
}

int wtouchln(WINDOW *win, int y, int n, int changed)
{
    if (!win || y < 0 || n <= 0 || y >= win->maxy)
        return -1;
    if (changed)
        mark_dirty(win, y, n);
    else
        memset(&win->dirty[y], 0, (size_t)(n > win->maxy - y ? win->maxy - y : n));
    return 0;
}

int touchwin(WINDOW *win)
{
    if (!win)
        return -1;
    mark_dirty(win, 0, win->maxy);
    return 0;
}

int redrawwin(WINDOW *win)
{
    return touchwin(win);
}
