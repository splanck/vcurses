#ifndef CURSES_H
#define CURSES_H

#include "vcurses.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif



extern WINDOW *stdscr;
extern int LINES;
extern int COLS;

WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x);
int delwin(WINDOW *win);
WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
WINDOW *derwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
WINDOW *newpad(int nlines, int ncols);
WINDOW *subpad(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
int prefresh(WINDOW *pad, int pminrow, int pmincol,
             int sminrow, int smincol, int smaxrow, int smaxcol);
int wmove(WINDOW *win, int y, int x);
int move(int y, int x);
int waddch(WINDOW *win, char ch);
int addch(char ch);
int addstr(const char *str);
int waddstr(WINDOW *win, const char *str);
int wgetch(WINDOW *win);
int getch(void);
int wgetstr(WINDOW *win, char *buf);
int getstr(char *buf);
int keypad(WINDOW *win, bool yes);
int nodelay(WINDOW *win, bool bf);
int wtimeout(WINDOW *win, int delay);
int halfdelay(int tenths);
int scrollok(WINDOW *win, bool bf);
int wscrl(WINDOW *win, int lines);
int wborder(WINDOW *win,
            char ls, char rs, char ts, char bs,
            char tl, char tr, char bl, char br);
int box(WINDOW *win, char verch, char horch);

/* --- Mouse support ---------------------------------------------------- */
typedef unsigned long mmask_t;

typedef struct {
    short id;
    int x, y, z;
    mmask_t bstate;
} MEVENT;

#define BUTTON1_PRESSED    (1UL << 0)
#define BUTTON1_RELEASED   (1UL << 1)
#define BUTTON2_PRESSED    (1UL << 2)
#define BUTTON2_RELEASED   (1UL << 3)
#define BUTTON3_PRESSED    (1UL << 4)
#define BUTTON3_RELEASED   (1UL << 5)
#define BUTTON4_PRESSED    (1UL << 6)
#define BUTTON4_RELEASED   (1UL << 7)
#define BUTTON5_PRESSED    (1UL << 8)
#define BUTTON5_RELEASED   (1UL << 9)
#define BUTTON_SHIFT       (1UL << 10)
#define BUTTON_CTRL        (1UL << 11)
#define BUTTON_ALT         (1UL << 12)
#define REPORT_MOUSE_POSITION (1UL << 13)

#define KEY_MOUSE 0x200

mmask_t mousemask(mmask_t newmask, mmask_t *oldmask);
int getmouse(MEVENT *event);

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
int pair_content(short pair, short *fg, short *bg);
int color_content(short color, short *r, short *g, short *b);

int attron(int attrs);
int attroff(int attrs);
int attrset(int attrs);
int wattron(WINDOW *win, int attrs);
int wattroff(WINDOW *win, int attrs);
int wattrset(WINDOW *win, int attrs);
int color_set(short pair, void *opts);
int wcolor_set(WINDOW *win, short pair, void *opts);
int curs_set(int visibility);
int beep(void);
int flash(void);

/* Internal helper used by the library */
void _vcurses_apply_attr(int attr);

#ifdef __cplusplus
}
#endif

#endif /* CURSES_H */
