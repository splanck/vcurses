# vcurses Reference

vcurses is a minimalist, modern reimplementation of a small subset of the
[ncurses](https://invisible-island.net/ncurses/) API.  It provides window
management, input handling and basic color/attribute support for terminal
applications.  The focus of the project is portability and simplicity.

## Building

vcurses is built with `make` using a standard C compiler.  Build the static
library `libvcurses.a` with:

```sh
make
```

This produces the library in the project root.  Remove build artefacts with
`make clean`.

## Initialization and shutdown

```c
WINDOW *initscr(void);
int endwin(void);
```

`initscr()` prepares the terminal for vcurses.  It switches to raw mode,
disables echo and allocates the global window `stdscr`.  The screen size is
accessible through the variables `LINES` and `COLS`.  Use `endwin()` to restore
the terminal state before exiting.

The helper `vc_init()` currently performs no work but is provided for
completeness.

## Basic screen functions

```c
int clear(void);
int clrtobot(void);
int clrtoeol(void);
int refresh(void);
int curs_set(int visibility);
```

`clear()` erases the screen buffer. `clrtobot()` and `clrtoeol()` clear regions
relative to the cursor in `stdscr`.  `refresh()` flushes the internal buffer to
the terminal.  Cursor visibility can be toggled with `curs_set()` which accepts
`0` to hide or `1` to show the cursor.

## Windows

```c
WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x);
int delwin(WINDOW *win);
WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
WINDOW *derwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
int wmove(WINDOW *win, int y, int x);
int move(int y, int x); /* acts on stdscr */
int wborder(WINDOW *win,
            char ls, char rs, char ts, char bs,
            char tl, char tr, char bl, char br);
int box(WINDOW *win, char verch, char horch);
int scrollok(WINDOW *win, bool bf);
int wscrl(WINDOW *win, int lines);
```

Windows provide independent regions of the screen.  `subwin()` and `derwin()`
create windows relative to an existing parent.  `wmove()` moves the window
cursor; `move()` is the stdscr variant.  Borders can be drawn with `wborder()`
or the convenience `box()` function.  Scrolling is enabled per window with
`scrollok()` and performed using `wscrl()`.

## Output functions

```c
int waddch(WINDOW *win, char ch);
int addch(char ch);                 /* stdscr */
int waddstr(WINDOW *win, const char *str);
int addstr(const char *str);        /* stdscr */
```

Characters and strings are placed at the current cursor location of a window.

## Input

```c
int wgetch(WINDOW *win);
int getch(void);            /* stdscr */
int keypad(WINDOW *win, bool yes);
int echo(void);
int noecho(void);
int cbreak(void);
int nocbreak(void);
```

`getch()` and `wgetch()` read a character.  When keypad mode is enabled with
`keypad(win, true)` arrow keys and function keys are returned as `KEY_*`
constants such as `KEY_UP` or `KEY_F1`.  Echoing of typed characters and
canonical input processing can be toggled with the `echo`/`noecho` and
`cbreak`/`nocbreak` calls.

## Colors and attributes

```c
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
```

Color support provides eight base colors (`COLOR_BLACK` .. `COLOR_WHITE`) and up
to 256 color pairs.  Call `start_color()` once after initialization then define
pairs with `init_pair()`.  Attributes include `A_BOLD` and `A_UNDERLINE`.
`COLOR_PAIR(n)` composes an attribute mask from a pair number which can be set
with `attron()`/`wattron()` or `color_set()`.

## Example

The following program prints styled text using color pairs and attributes:

```c
#include "curses.h"
#include <unistd.h>

int main(void) {
    if (initscr() == NULL)
        return 1;

    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);

    wmove(stdscr, 0, 0);
    wattron(stdscr, A_BOLD);
    waddstr(stdscr, "Bold text\n");
    wattroff(stdscr, A_BOLD);

    wmove(stdscr, 1, 0);
    wattron(stdscr, A_UNDERLINE);
    waddstr(stdscr, "Underlined text\n");
    wattroff(stdscr, A_UNDERLINE);

    wmove(stdscr, 2, 0);
    wattron(stdscr, A_BOLD | A_UNDERLINE);
    wcolor_set(stdscr, 1, NULL);
    waddstr(stdscr, "Bold, Underlined & Yellow\n");
    wattroff(stdscr, A_BOLD | A_UNDERLINE);
    wcolor_set(stdscr, 0, NULL);

    refresh();
    sleep(2);
    endwin();
    return 0;
}
```

Refer to the programs in `demos/` for additional usage patterns including window
management, input handling and screen resizing.

## Man pages

Detailed manual pages for several functions are located in `docs/man` and can be
viewed with `man -l docs/man/initscr.3`.

