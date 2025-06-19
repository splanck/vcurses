#include "curses.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char **screen_buf = NULL;
static int buf_rows = 0;
static int buf_cols = 0;

static int ensure_buffer(void) {
    if (!stdscr)
        return -1;
    if (screen_buf && buf_rows == stdscr->maxy && buf_cols == stdscr->maxx)
        return 0;
    if (screen_buf) {
        for (int r = 0; r < buf_rows; ++r)
            free(screen_buf[r]);
        free(screen_buf);
        screen_buf = NULL;
    }
    buf_rows = stdscr->maxy;
    buf_cols = stdscr->maxx;
    screen_buf = malloc(sizeof(char *) * buf_rows);
    if (!screen_buf)
        return -1;
    for (int r = 0; r < buf_rows; ++r) {
        screen_buf[r] = malloc(buf_cols);
        if (!screen_buf[r]) {
            for (int i = 0; i < r; ++i)
                free(screen_buf[i]);
            free(screen_buf);
            screen_buf = NULL;
            return -1;
        }
        memset(screen_buf[r], ' ', buf_cols);
    }
    return 0;
}

void _vc_screen_puts(int y, int x, const char *str) {
    if (ensure_buffer() == -1 || !str)
        return;
    if (y < 0 || y >= buf_rows)
        return;
    int col = x;
    for (const char *p = str; *p && col < buf_cols; ++p)
        screen_buf[y][col++] = *p;
}

void _vc_screen_free(void) {
    if (!screen_buf)
        return;
    for (int r = 0; r < buf_rows; ++r)
        free(screen_buf[r]);
    free(screen_buf);
    screen_buf = NULL;
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
    for (int r = 0; r < buf_rows; ++r)
        memset(screen_buf[r], ' ', buf_cols);
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
    if (stdscr->curx < buf_cols)
        memset(&screen_buf[stdscr->cury][stdscr->curx], ' ', buf_cols - stdscr->curx);
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
    for (int r = stdscr->cury + 1; r < buf_rows; ++r)
        memset(screen_buf[r], ' ', buf_cols);
    return 0;
}

int refresh(void) {
    if (ensure_buffer() == -1)
        return fflush(stdout);
    for (int r = 0; r < buf_rows; ++r) {
        printf("\x1b[%d;1H", r + 1);
        fwrite(screen_buf[r], 1, buf_cols, stdout);
    }
    if (stdscr)
        printf("\x1b[%d;%dH", stdscr->cury + 1, stdscr->curx + 1);
    return fflush(stdout);
}
