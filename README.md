# vcurses

Minimalist modern curses compatible text library.

## Building

Ensure you have a C compiler (e.g. `cc`) and `make` installed. To build the
static library run:

```sh
make
```

This creates `libvcurses.a` in the project root. You can clean the build
artifacts with:

```sh
make clean
```

## Documentation

Man pages for the core API are available in `docs/man`. Useful entries
include [initscr.3](docs/man/initscr.3) and [getch.3](docs/man/getch.3).
Additional notes on reading strings with `getstr` are in
[vcursesdoc.md](vcursesdoc.md).
Pad usage is documented in the [Pads section](vcursesdoc.md#pads).

## Color support

`vcurses` provides eight basic colors named `COLOR_BLACK` through
`COLOR_WHITE`. Call `start_color()` once after initializing the library to
enable color handling and define up to 256 color pairs with
`init_pair(pair, fg, bg)`. The macro `COLOR_PAIR(n)` can then be used with
`attron`, `attroff`, or `attrset` (or their window variants) to apply a
defined pair. Use `pair_content(pair, &fg, &bg)` to query a pair and
`color_content(color, &r, &g, &b)` to retrieve the RGB components of a
color. Individual colors can be redefined with `init_color(color, r, g, b)`
after calling `start_color()`.

## Input timeouts

`wgetch()` can wait for input in three ways:

- `nodelay(win, true)` makes reads non-blocking.
- `wtimeout(win, ms)` specifies a delay in milliseconds for the next read.
- `halfdelay(tenths)` sets cbreak mode and applies a timeout on `stdscr`.

## Terminal modes

`initscr()` enters raw mode with echo disabled. Call `noraw()` to
restore normal signal handling and `raw()` to enable raw mode again.
Line buffering and echo can also be toggled with `cbreak()`/
`nocbreak()` and `echo()`/`noecho()`.

## Alerts

`beep()` writes an audible bell (\a) to the terminal. `flash()` emits a brief
visual flash using ANSI escape codes when supported.

`refresh()` redraws the entire screen from the internal buffer. Use
`wrefresh(win)` to update only a specific window when you don't need to
repaint everything.

## Clearing windows

`wclear(win)` erases all contents of a window. `wclrtoeol(win)` blanks
from the cursor to the end of the current line and `wclrtobot(win)` clears
from the cursor to the bottom of the window. These helpers modify only the
target window's backing buffers.

## Scrolling

Use `wscrl(win, lines)` to scroll a window explicitly. A positive `lines`
value scrolls the region up while a negative value scrolls it down. Newly
exposed lines are cleared using the window's current attributes.

## Mouse input

Mouse events can be enabled with `mousemask()` and read with `getmouse()` when
`wgetch()` returns `KEY_MOUSE`.

## Window coordinate macros

Several macros match the ncurses API for retrieving window positions. They
expand to simple field accesses:

```c
int y, x;
getyx(win, y, x);      /* cursor position */
getbegyx(win, y, x);   /* window origin */
getmaxyx(win, y, x);   /* window size */
```

Single-value helpers like `getcurx(win)` and `getmaxy(win)` are also provided.

## Formatted output

`wprintw(win, fmt, ...)` works like `printf` but writes into a window.
`printw()` is a convenience wrapper that targets `stdscr`.

```c
wprintw(win, "Count: %d", n);
printw("%s\n", msg);
```

