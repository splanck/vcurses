# vcurses Reference Guide

## Building

To build the static library ensure `make` and a C compiler are installed, then run:

```sh
make
```

This produces `libvcurses.a` in the project root. Clean build artifacts with:

```sh
make clean
```

To run the unit tests use:

```sh
make test
```

## Overview

`vcurses` is a small C library implementing a subset of the [ncurses](https://invisible-island.net/ncurses/) API. It exposes familiar window and screen functions for terminal user interfaces while keeping the implementation lightweight.

### Global variables

- `WINDOW *stdscr` – the default screen window
- `int LINES` and `int COLS` – current terminal size

### Initialization

```c
WINDOW *initscr(void);
int endwin(void);
```

Call `initscr()` to initialize the library and switch the terminal into raw mode. When finished, call `endwin()` to restore the terminal state.

### Basic screen functions

```c
int clear(void);
int clrtobot(void);
int clrtoeol(void);
int refresh(void);
int wnoutrefresh(WINDOW *win);
int noutrefresh(void);
int doupdate(void);
```

These helpers manipulate the virtual screen buffer. `wnoutrefresh()` queues
changes without drawing them so multiple windows can be updated before
calling `doupdate()` to refresh the terminal. `noutrefresh()` is a wrapper
for `stdscr`. `wrefresh(win)` simply performs `wnoutrefresh(win)` followed
by `doupdate()`.

### Terminal modes

```c
echo();    /* enable input echo */
noecho();  /* disable echo */
cbreak();  /* disable line buffering */
nocbreak();
raw();     /* disable signals and line buffering */
noraw();
```

`initscr()` sets raw mode with no echo by default. Use `noraw()` to
restore normal signal processing and `raw()` to enable it again. The
`cbreak()` and `nocbreak()` helpers only toggle line buffering.

### Alerts

```c
int beep(void);
int flash(void);
```

`beep()` outputs an audible bell. `flash()` triggers a visual bell using ANSI
escape sequences when supported.

### Windows

Window creation and movement routines:

```c
WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x);
int delwin(WINDOW *win);
WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
WINDOW *derwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
int wmove(WINDOW *win, int y, int x);
int move(int y, int x); /* stdscr wrapper */
```

Text output primitives:

```c
int waddch(WINDOW *win, char ch);
int waddstr(WINDOW *win, const char *str);
int addch(char ch);      /* stdscr wrapper */
int addstr(const char *str);
```

Scrolling, borders and line drawing:

```c
int scrollok(WINDOW *win, bool bf);
int wscrl(WINDOW *win, int lines);
int box(WINDOW *win, char verch, char horch);
int wborder(WINDOW *win,
            char ls, char rs, char ts, char bs,
            char tl, char tr, char bl, char br);
int whline(WINDOW *win, char ch, int n);
int hline(char ch, int n);
int wvline(WINDOW *win, char ch, int n);
int vline(char ch, int n);
```

### Input

```c
int wgetch(WINDOW *win);
int getch(void);          /* stdscr wrapper */
int keypad(WINDOW *win, bool yes);
```

When keypad mode is enabled, arrow keys, function keys and other special keys are returned as predefined `KEY_*` codes such as `KEY_UP` or `KEY_F1`.

Input blocking behaviour can be tuned per window:

```c
int nodelay(WINDOW *win, bool bf);  /* true for non-blocking */
int wtimeout(WINDOW *win, int delay); /* milliseconds, -1 blocking */
int timeout(int delay); /* stdscr wrapper */
int halfdelay(int tenths); /* sets stdscr timeout */
int set_escdelay(int ms); /* ESC detection delay */
int meta(WINDOW *win, bool bf); /* return 8-bit input */
```

`nodelay()` is shorthand for `wtimeout(win, 0)`.  `timeout()` is the same operation on `stdscr`. Positive values passed to `wtimeout()` specify the maximum wait time in milliseconds.  `halfdelay()` enables cbreak mode and waits up to the given tenths of a second when reading from `stdscr`.  `set_escdelay()` controls how long `wgetch()` waits for the remainder of an escape sequence before returning the ESC key itself.
`meta(win, true)` enables 8-bit input so bytes above 0x7F are returned unchanged. With it disabled the high bit is stripped.

### Attributes and colors

Color and attribute management helpers:

```c
int start_color(void);
int init_pair(short pair, short fg, short bg);
int pair_content(short pair, short *fg, short *bg);
int color_content(short color, short *r, short *g, short *b);
int attron(int attrs);  int wattron(WINDOW *win, int attrs);
int attroff(int attrs); int wattroff(WINDOW *win, int attrs);
int attrset(int attrs); int wattrset(WINDOW *win, int attrs);
int color_set(short pair, void *opts);  /* stdscr wrapper */
int wcolor_set(WINDOW *win, short pair, void *opts);
```

The header defines color constants and attribute masks:

```c
#define COLOR_BLACK   0
#define COLOR_RED     1
#define COLOR_GREEN   2
#define COLOR_YELLOW  3
#define COLOR_BLUE    4
#define COLOR_MAGENTA 5
#define COLOR_CYAN    6
#define COLOR_WHITE   7

#define COLOR_PAIR(n)  ((n) << 8)
#define PAIR_NUMBER(a) (((a) >> 8) & 0xFF)

#define A_NORMAL      0x0000
#define A_COLOR       0xFF00
#define A_BOLD        0x010000
#define A_UNDERLINE   0x020000
#define A_REVERSE     0x040000
#define A_BLINK       0x080000
#define A_DIM         0x100000
#define A_STANDOUT    0x200000
```

Use `start_color()` once after initialization, define pairs with `init_pair()` and query them with `pair_content()`. The RGB components of the basic colors can be obtained using `color_content()`. Apply pairs with the attribute functions above.

### Cursor visibility

```c
int curs_set(int visibility);
```

Pass `0` to hide the cursor or `1` to show it. The previous state is returned.

## Examples

### Hello world

```c
#include "curses.h"
#include <unistd.h>

int main(void) {
    if (initscr() == NULL)
        return 1;

    addstr("Hello, world!\n");
    refresh();
    sleep(2);

    endwin();
    return 0;
}
```

Compile with:

```sh
cc hello.c libvcurses.a -o hello
```

### Colored output

```c
#include "curses.h"
#include <unistd.h>

int main(void) {
    initscr();
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);

    wattron(stdscr, A_BOLD);
    wcolor_set(stdscr, 1, NULL);
    waddstr(stdscr, "Red bold text\n");
    wattroff(stdscr, A_BOLD);
    wcolor_set(stdscr, 0, NULL);

    refresh();
    sleep(2);
    endwin();
    return 0;
}
```

This program demonstrates attributes and color pairs.

---

For further details consult the man pages in `docs/man`, for example `man docs/man/initscr.3`.
