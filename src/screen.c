#include "curses.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char **screen_buf = NULL;
static int **attr_buf = NULL;
static int buf_rows = 0;
static int buf_cols = 0;

static int ensure_buffer(void) {
    if (!stdscr)
        return -1;
    if (screen_buf && attr_buf &&
        buf_rows == stdscr->maxy && buf_cols == stdscr->maxx)
        return 0;
    if (screen_buf) {
        for (int r = 0; r < buf_rows; ++r)
            free(screen_buf[r]);
        free(screen_buf);
        screen_buf = NULL;
    }
    if (attr_buf) {
        for (int r = 0; r < buf_rows; ++r)
            free(attr_buf[r]);
        free(attr_buf);
        attr_buf = NULL;
    }
    buf_rows = stdscr->maxy;
    buf_cols = stdscr->maxx;
    screen_buf = malloc(sizeof(char *) * buf_rows);
    attr_buf = malloc(sizeof(int *) * buf_rows);
    if (!screen_buf || !attr_buf)
        return -1;
    for (int r = 0; r < buf_rows; ++r) {
        screen_buf[r] = malloc(buf_cols);
        attr_buf[r] = malloc(sizeof(int) * buf_cols);
        if (!screen_buf[r] || !attr_buf[r]) {
            for (int i = 0; i <= r; ++i) {
                if (i < r) {
                    free(screen_buf[i]);
                    free(attr_buf[i]);
                }
            }
            free(screen_buf);
            free(attr_buf);
            screen_buf = NULL;
            attr_buf = NULL;
            return -1;
        }
        memset(screen_buf[r], ' ', buf_cols);
        for (int c = 0; c < buf_cols; ++c)
            attr_buf[r][c] = stdscr->bkgd;
    }
    return 0;
}

void _vc_screen_puts(int y, int x, const char *str, int attr) {
    if (ensure_buffer() == -1 || !str)
        return;
    if (y < 0 || y >= buf_rows)
        return;
    int col = x;
    for (const char *p = str; *p && col < buf_cols; ++p) {
        screen_buf[y][col] = *p;
        attr_buf[y][col] = attr;
        col++;
    }
}

void _vc_screen_free(void) {
    if (screen_buf) {
        for (int r = 0; r < buf_rows; ++r)
            free(screen_buf[r]);
        free(screen_buf);
        screen_buf = NULL;
    }
    if (attr_buf) {
        for (int r = 0; r < buf_rows; ++r)
            free(attr_buf[r]);
        free(attr_buf);
        attr_buf = NULL;
    }
}

int clear(void) {
    if (ensure_buffer() == -1) {
        fputs("\x1b[2J\x1b[H", stdout);
        if (stdscr) {
            stdscr->cury = 0;
            stdscr->curx = 0;
        }
        return 0;
    }
    for (int r = 0; r < buf_rows; ++r) {
        memset(screen_buf[r], ' ', buf_cols);
        for (int c = 0; c < buf_cols; ++c)
            attr_buf[r][c] = stdscr->bkgd;
    }
    if (stdscr) {
        stdscr->cury = 0;
        stdscr->curx = 0;
    }
    return 0;
}

int clrtoeol(void) {
    if (ensure_buffer() == -1) {
        fputs("\x1b[K", stdout);
        return 0;
    }
    if (!stdscr || stdscr->cury >= buf_rows)
        return -1;
    if (stdscr->curx < buf_cols) {
        memset(&screen_buf[stdscr->cury][stdscr->curx], ' ', buf_cols - stdscr->curx);
        for (int c = stdscr->curx; c < buf_cols; ++c)
            attr_buf[stdscr->cury][c] = stdscr->bkgd;
    }
    return 0;
}

int clrtobot(void) {
    if (ensure_buffer() == -1) {
        fputs("\x1b[J", stdout);
        return 0;
    }
    if (!stdscr || stdscr->cury >= buf_rows)
        return -1;
    clrtoeol();
    for (int r = stdscr->cury + 1; r < buf_rows; ++r) {
        memset(screen_buf[r], ' ', buf_cols);
        for (int c = 0; c < buf_cols; ++c)
            attr_buf[r][c] = stdscr->bkgd;
    }
    return 0;
}

int refresh(void) {
    if (ensure_buffer() == -1)
        return fflush(stdout);
    for (int r = 0; r < buf_rows; ++r) {
        printf("\x1b[%d;1H", r + 1);
        int current_attr = -1;
        for (int c = 0; c < buf_cols; ++c) {
            int a = attr_buf[r][c];
            if (a != current_attr) {
                _vcurses_apply_attr(a);
                current_attr = a;
            }
            fputc(screen_buf[r][c], stdout);
        }
        if (current_attr != -1)
            _vcurses_apply_attr(A_NORMAL | COLOR_PAIR(0));
    }
    if (stdscr)
        printf("\x1b[%d;%dH", stdscr->cury + 1, stdscr->curx + 1);
    return fflush(stdout);
}

void _vc_screen_refresh_region(int top, int left, int height, int width) {
    if (ensure_buffer() == -1)
        return;
    if (top < 0 || left < 0 || top >= buf_rows || left >= buf_cols)
        return;
    int bottom = top + height;
    if (bottom > buf_rows)
        bottom = buf_rows;
    int right = left + width;
    if (right > buf_cols)
        right = buf_cols;

    for (int r = top; r < bottom; ++r) {
        printf("\x1b[%d;%dH", r + 1, left + 1);
        int current_attr = -1;
        for (int c = left; c < right; ++c) {
            int a = attr_buf[r][c];
            if (a != current_attr) {
                _vcurses_apply_attr(a);
                current_attr = a;
            }
            fputc(screen_buf[r][c], stdout);
        }
        if (current_attr != -1)
            _vcurses_apply_attr(A_NORMAL | COLOR_PAIR(0));
    }
}

void _vc_screen_scroll_region(int top, int left, int height, int width,
                              int lines, int attr) {
    /* scroll a rectangular region by the given line count.  A positive
       count scrolls up while a negative count scrolls down. */
    if (ensure_buffer() == -1)
        return;
    if (lines == 0 || height <= 0 || width <= 0)
        return;
    if (top < 0 || left < 0 || top + height > buf_rows || left + width > buf_cols)
        return;

    int count = lines > 0 ? lines : -lines;
    if (count > height)
        count = height;

    if (lines > 0) {
        for (int r = 0; r < height - count; ++r) {
            memmove(&screen_buf[top + r][left],
                    &screen_buf[top + r + count][left], width);
            memmove(&attr_buf[top + r][left],
                    &attr_buf[top + r + count][left], sizeof(int) * width);
        }
        for (int r = height - count; r < height; ++r) {
            memset(&screen_buf[top + r][left], ' ', width);
            for (int c = 0; c < width; ++c)
                attr_buf[top + r][left + c] = attr;
        }
    } else {
        for (int r = height - 1; r >= count; --r) {
            memmove(&screen_buf[top + r][left],
                    &screen_buf[top + r - count][left], width);
            memmove(&attr_buf[top + r][left],
                    &attr_buf[top + r - count][left], sizeof(int) * width);
        }
        for (int r = 0; r < count; ++r) {
            memset(&screen_buf[top + r][left], ' ', width);
            for (int c = 0; c < width; ++c)
                attr_buf[top + r][left + c] = attr;
        }
    }
}

int _vc_screen_get_cell(int y, int x, char *ch, int *attr)
{
    if (ensure_buffer() == -1)
        return -1;
    if (y < 0 || y >= buf_rows || x < 0 || x >= buf_cols)
        return -1;
    if (ch)
        *ch = screen_buf[y][x];
    if (attr)
        *attr = attr_buf[y][x];
    return 0;
}
