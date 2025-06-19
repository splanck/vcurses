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
    int keypad_mode; /* keypad enabled */
    int attr; /* current attributes */
} WINDOW;

extern WINDOW *stdscr;

WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x);
int delwin(WINDOW *win);
WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
WINDOW *derwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
int wmove(WINDOW *win, int y, int x);
int waddstr(WINDOW *win, const char *str);
int wgetch(WINDOW *win);
int getch(void);
int keypad(WINDOW *win, int bf);

/* Special key definitions */
#define KEY_UP     0x101
#define KEY_DOWN   0x102
#define KEY_LEFT   0x103
#define KEY_RIGHT  0x104
#define KEY_HOME   0x105
#define KEY_END    0x106
#define KEY_NPAGE  0x107
#define KEY_PPAGE  0x108
#define KEY_IC     0x109
#define KEY_DC     0x10A

#define KEY_F0     0x110
#define KEY_F(n)   (KEY_F0 + (n))
#define KEY_F1     KEY_F(1)
#define KEY_F2     KEY_F(2)
#define KEY_F3     KEY_F(3)
#define KEY_F4     KEY_F(4)
#define KEY_F5     KEY_F(5)
#define KEY_F6     KEY_F(6)
#define KEY_F7     KEY_F(7)
#define KEY_F8     KEY_F(8)
#define KEY_F9     KEY_F(9)
#define KEY_F10    KEY_F(10)
#define KEY_F11    KEY_F(11)
#define KEY_F12    KEY_F(12)

/* Color definitions */
#define COLOR_BLACK   0
#define COLOR_RED     1
#define COLOR_GREEN   2
#define COLOR_YELLOW  3
#define COLOR_BLUE    4
#define COLOR_MAGENTA 5
#define COLOR_CYAN    6
#define COLOR_WHITE   7

#define COLOR_PAIRS   256
#define COLOR_PAIR(n)   ((n) << 8)
#define PAIR_NUMBER(a)  (((a) >> 8) & 0xFF)

/* Attribute masks */
#define A_NORMAL      0x0000
#define A_COLOR       0xFF00
#define A_BOLD        0x010000
#define A_UNDERLINE   0x020000

int start_color(void);
int init_pair(short pair, short fg, short bg);

int attron(int attrs);
int attroff(int attrs);
int attrset(int attrs);
int wattron(WINDOW *win, int attrs);
int wattroff(WINDOW *win, int attrs);
int wattrset(WINDOW *win, int attrs);
int color_set(short pair, void *opts);
int wcolor_set(WINDOW *win, short pair, void *opts);

/* Internal helper used by the library */
void _vcurses_apply_attr(int attr);

#ifdef __cplusplus
}
#endif

#endif /* CURSES_H */
