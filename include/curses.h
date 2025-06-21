#ifndef CURSES_H
#define CURSES_H

#include "vcurses.h"
#include <stdbool.h>
#include <wchar.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif



extern WINDOW *stdscr;
extern int LINES;
extern int COLS;

struct refresh_rect;
struct win_node;

typedef struct screen {
    WINDOW *stdscr;
    int lines, cols;
    char **screen_buf;
    int **attr_buf;
    int buf_rows, buf_cols;
    struct refresh_rect *refresh_head;
    int cursor_y, cursor_x;
    struct win_node *win_list;
} SCREEN;

extern SCREEN *_vc_current_screen;
SCREEN *newterm(const char *type, FILE *outf, FILE *inf);
SCREEN *set_term(SCREEN *scr);

WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x);
int delwin(WINDOW *win);
WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
WINDOW *derwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
int mvwin(WINDOW *win, int y, int x);
/* Pad management */
WINDOW *newpad(int nlines, int ncols);
WINDOW *subpad(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
int prefresh(WINDOW *pad, int pminrow, int pmincol,
             int sminrow, int smincol, int smaxrow, int smaxcol);
int wmove(WINDOW *win, int y, int x);
int move(int y, int x);
int waddch(WINDOW *win, char ch);
int addch(char ch);
int addstr(const char *str);
int addnstr(const char *str, int n);
int waddstr(WINDOW *win, const char *str);
int waddnstr(WINDOW *win, const char *str, int n);
int wadd_wch(WINDOW *win, wchar_t wch);
int add_wch(wchar_t wch);
int mvwaddch(WINDOW *win, int y, int x, char ch);
int mvaddch(int y, int x, char ch);
int mvwadd_wch(WINDOW *win, int y, int x, wchar_t wch);
int mvadd_wch(int y, int x, wchar_t wch);
int mvwaddstr(WINDOW *win, int y, int x, const char *str);
int mvaddstr(int y, int x, const char *str);
int mvwaddnstr(WINDOW *win, int y, int x, const char *str, int n);
int mvaddnstr(int y, int x, const char *str, int n);
int wprintw(WINDOW *win, const char *fmt, ...);
int printw(const char *fmt, ...);
int mvwprintw(WINDOW *win, int y, int x, const char *fmt, ...);
int mvprintw(int y, int x, const char *fmt, ...);
int wgetch(WINDOW *win);
int getch(void);
int ungetch(int ch);
int wget_wch(WINDOW *win, wchar_t *wch);
int get_wch(wchar_t *wch);
int unget_wch(wchar_t wch);
int wgetstr(WINDOW *win, char *buf);
int getstr(char *buf);
int wgetnstr(WINDOW *win, char *buf, int n);
int getnstr(char *buf, int n);
int wscanw(WINDOW *win, const char *fmt, ...);
int scanw(const char *fmt, ...);
int mvwscanw(WINDOW *win, int y, int x, const char *fmt, ...);
int mvscanw(int y, int x, const char *fmt, ...);
const char *keyname(int ch);
int has_key(int keycode);
int keypad(WINDOW *win, bool yes);
int nodelay(WINDOW *win, bool bf);
int notimeout(WINDOW *win, bool bf);
int meta(WINDOW *win, bool bf);
int wtimeout(WINDOW *win, int delay);
int timeout(int delay);
int halfdelay(int tenths);
int set_escdelay(int ms);
int flushinp(void);
int leaveok(WINDOW *win, bool bf);
int scrollok(WINDOW *win, bool bf);
int clearok(WINDOW *win, bool bf);
int wsetscrreg(WINDOW *win, int top, int bottom);
int setscrreg(int top, int bottom);
int wscrl(WINDOW *win, int lines);
int scroll(WINDOW *win);
int wborder(WINDOW *win,
            char ls, char rs, char ts, char bs,
            char tl, char tr, char bl, char br);
int box(WINDOW *win, char verch, char horch);
int whline(WINDOW *win, char ch, int n);
int hline(char ch, int n);
int wvline(WINDOW *win, char ch, int n);
int vline(char ch, int n);
int wresize(WINDOW *win, int nlines, int ncols);
int resizeterm(int lines, int cols);
int wrefresh(WINDOW *win);
int wnoutrefresh(WINDOW *win);
int noutrefresh(void);
int doupdate(void);
int wtouchln(WINDOW *win, int y, int n, int changed);
int touchwin(WINDOW *win);
int redrawwin(WINDOW *win);
int werase(WINDOW *win);
int wclear(WINDOW *win);
int wclrtobot(WINDOW *win);
int wclrtoeol(WINDOW *win);
int winsdelln(WINDOW *win, int n);
int insertln(void);
int deleteln(void);
int winsch(WINDOW *win, char ch);
int mvwinsch(WINDOW *win, int y, int x, char ch);
int insch(char ch);
int mvinsch(int y, int x, char ch);
int wdelch(WINDOW *win);
int mvwdelch(WINDOW *win, int y, int x);
int delch(void);
int mvdelch(int y, int x);
int winsstr(WINDOW *win, const char *str);
int mvwinsstr(WINDOW *win, int y, int x, const char *str);
int insstr(const char *str);
int mvinsstr(int y, int x, const char *str);
int copywin(WINDOW *src, WINDOW *dst,
            int sminrow, int smincol,
            int dminrow, int dmincol,
            int dmaxrow, int dmaxcol,
            int overlay);
int overlay(WINDOW *src, WINDOW *dst);
int overwrite(WINDOW *src, WINDOW *dst);
int erase(void);

/* --- Convenience macros ---------------------------------------------- */
#define getcury(win)      ((win)->cury)
#define getcurx(win)      ((win)->curx)
#define getbegy(win)      ((win)->begy)
#define getbegx(win)      ((win)->begx)
#define getmaxy(win)      ((win)->maxy)
#define getmaxx(win)      ((win)->maxx)
#define getpary(win)      ((win)->parent ? (win)->begy - (win)->parent->begy : -1)
#define getparx(win)      ((win)->parent ? (win)->begx - (win)->parent->begx : -1)
#define getyx(win,y,x)    ((y) = getcury(win), (x) = getcurx(win))
#define getbegyx(win,y,x) ((y) = getbegy(win), (x) = getbegx(win))
#define getmaxyx(win,y,x) ((y) = getmaxy(win), (x) = getmaxx(win))
#define getparyx(win,y,x) ((y) = getpary(win), (x) = getparx(win))

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
#define KEY_RESIZE 0x201

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
#define KEY_BACKSPACE 0x10B
#define KEY_ENTER     0x10C

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

/* Alternate character set constants */
#define ACS_ULCORNER  0x80
#define ACS_LLCORNER  0x81
#define ACS_URCORNER  0x82
#define ACS_LRCORNER  0x83
#define ACS_LTEE      0x84
#define ACS_RTEE      0x85
#define ACS_TTEE      0x86
#define ACS_BTEE      0x87
#define ACS_HLINE     0x88
#define ACS_VLINE     0x89
#define ACS_PLUS      0x8A
#define ACS_S1        0x8B
#define ACS_S9        0x8C
#define ACS_DIAMOND   0x8D
#define ACS_CKBOARD   0x8E
#define ACS_DEGREE    0x8F
#define ACS_PLMINUS   0x90
#define ACS_BULLET    0x91
#define ACS_LARROW    0x92
#define ACS_RARROW    0x93
#define ACS_UARROW    0x94
#define ACS_DARROW    0x95
#define ACS_BOARD     0x96
#define ACS_LANTERN   0x97
#define ACS_BLOCK     0x98


int start_color(void);
int use_default_colors(void);
int init_pair(short pair, short fg, short bg);
int pair_content(short pair, short *fg, short *bg);
int color_content(short color, short *r, short *g, short *b);
int init_color(short color, short r, short g, short b);
int has_colors(void);
int can_change_color(void);

int attron(int attrs);
int attroff(int attrs);
int attrset(int attrs);
int wattron(WINDOW *win, int attrs);
int wattroff(WINDOW *win, int attrs);
int wattrset(WINDOW *win, int attrs);
int color_set(short pair, void *opts);
int wcolor_set(WINDOW *win, short pair, void *opts);
int wbkgdset(WINDOW *win, int attrs);
int bkgdset(int attrs);
int wbkgd(WINDOW *win, int attrs);
int bkgd(int attrs);
int curs_set(int visibility);
int nl(void);
int nonl(void);
int beep(void);
int flash(void);

/* Internal helper used by the library */
void _vcurses_apply_attr(int attr);

#ifdef __cplusplus
}
#endif

#endif /* CURSES_H */
