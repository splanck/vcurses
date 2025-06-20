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

## Color support

`vcurses` provides eight basic colors named `COLOR_BLACK` through
`COLOR_WHITE`. Call `start_color()` once after initializing the library to
enable color handling and define up to 256 color pairs with
`init_pair(pair, fg, bg)`. The macro `COLOR_PAIR(n)` can then be used with
`attron`, `attroff`, or `attrset` (or their window variants) to apply a
defined pair. Use `pair_content(pair, &fg, &bg)` to query a pair and
`color_content(color, &r, &g, &b)` to retrieve the RGB components of a
color.

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

