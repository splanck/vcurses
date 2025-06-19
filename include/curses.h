#ifndef CURSES_H
#define CURSES_H

#include "vcurses.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif



extern WINDOW *stdscr;

WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x);
int delwin(WINDOW *win);
WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
WINDOW *derwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
int wmove(WINDOW *win, int y, int x);
int move(int y, int x);
int waddch(WINDOW *win, char ch);
int addch(char ch);
int addstr(const char *str);
int waddstr(WINDOW *win, const char *str);
int wgetch(WINDOW *win);
int getch(void);
int keypad(WINDOW *win, bool yes);

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
