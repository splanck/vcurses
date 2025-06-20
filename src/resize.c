#include "curses.h"
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

int LINES = 0;
int COLS = 0;

/* Linked list of all created windows */
struct win_node {
    WINDOW *win;
    struct win_node *next;
};
static struct win_node *win_list = NULL;

void _vc_register_window(WINDOW *win)
{
    struct win_node *n = malloc(sizeof(struct win_node));
    if (!n)
        return;
    n->win = win;
    n->next = win_list;
    win_list = n;
}

void _vc_unregister_window(WINDOW *win)
{
    struct win_node **cur = &win_list;
    while (*cur) {
        if ((*cur)->win == win) {
            struct win_node *tmp = *cur;
            *cur = tmp->next;
            free(tmp);
            return;
        }
        cur = &(*cur)->next;
    }
}

static void resize_all(int rows, int cols)
{
    LINES = rows;
    COLS = cols;
    if (stdscr) {
        if (stdscr->cury >= rows) stdscr->cury = rows ? rows - 1 : 0;
        if (stdscr->curx >= cols) stdscr->curx = cols ? cols - 1 : 0;
        stdscr->maxy = rows;
        stdscr->maxx = cols;
    }
    for (struct win_node *n = win_list; n; n = n->next) {
        WINDOW *w = n->win;
        if (!w || w == stdscr)
            continue;
        if (w->begy >= rows)
            w->begy = rows ? rows - 1 : 0;
        if (w->begx >= cols)
            w->begx = cols ? cols - 1 : 0;
        if (w->begy + w->maxy > rows)
            w->maxy = rows - w->begy;
        if (w->begx + w->maxx > cols)
            w->maxx = cols - w->begx;
        if (w->cury >= w->maxy)
            w->cury = w->maxy ? w->maxy - 1 : 0;
        if (w->curx >= w->maxx)
            w->curx = w->maxx ? w->maxx - 1 : 0;
    }
}

static void handle_winch(int sig)
{
    (void)sig;
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        resize_all(ws.ws_row, ws.ws_col);
    }
    ungetch(KEY_RESIZE);
}

void _vc_resize_init(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        resize_all(ws.ws_row, ws.ws_col);
    } else {
        resize_all(24, 80);
    }
    signal(SIGWINCH, handle_winch);
}

void _vc_resize_shutdown(void)
{
    signal(SIGWINCH, SIG_DFL);
    while (win_list) {
        struct win_node *tmp = win_list;
        win_list = tmp->next;
        free(tmp);
    }
}

extern void _vc_screen_free(void);
int resizeterm(int lines, int cols)
{
    if (lines <= 0 || cols <= 0)
        return -1;
    resize_all(lines, cols);
    ungetch(KEY_RESIZE);
    _vc_screen_free();
    return 0;
}

