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
after calling `start_color()`.  Calling `use_default_colors()` lets
`init_pair` accept `-1` as either color to keep the terminal's default
foreground or background.

## Text attributes

Attributes modify how text is displayed. `vcurses` supports
`A_BOLD`, `A_UNDERLINE`, `A_REVERSE`, `A_BLINK`, `A_DIM` and `A_STANDOUT`.
They can be combined with color pairs using `attron`/`wattron` and disabled
with `attroff`/`wattroff`.
Reverse video swaps the foreground and background colors:

```c
wattron(win, A_REVERSE);
waddstr(win, "reversed\n");
wattroff(win, A_REVERSE);
```

Each window also stores a background attribute used when blanks are inserted
by erasing or scrolling.  Set it with `wbkgdset(win, attr)` (or `bkgdset` for
`stdscr`). Calling `wbkgd(win, attr)` updates the background and clears the
window using the new attribute.

## Input timeouts

`wgetch()` can wait for input in three ways:

- `nodelay(win, true)` makes reads non-blocking.
- `wtimeout(win, ms)` specifies a delay in milliseconds for the next read.
- `halfdelay(tenths)` sets cbreak mode and applies a timeout on `stdscr`.
- `ungetch(ch)` pushes a character back so the next `getch` returns it.
Use `getnstr()` or `wgetnstr()` to stop reading a string once a fixed
length has been reached.

When keypad mode is enabled with `keypad(win, true)`, `wgetch()` maps
special keys to predefined constants such as `KEY_UP`, `KEY_F1`,
`KEY_BACKSPACE` and `KEY_ENTER`.  A full list is available in
[vcursesdoc.md](vcursesdoc.md#key-codes).

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

If the terminal contents become corrupted, `touchwin(win)` or
`wtouchln(win, line, count, 1)` mark regions of a window as dirty so the next
`wrefresh()` repaints them. `redrawwin(win)` is equivalent to touching the
whole window.

## Clearing windows

`wclear(win)` erases all contents of a window. `werase(win)` performs the same
blanking without forcing a full terminal clear. The convenience wrapper
`erase()` operates on `stdscr`. `wclrtoeol(win)` blanks from the cursor to the
end of the current line and `wclrtobot(win)` clears from the cursor to the
bottom of the window. These helpers modify only the target window's backing
buffers.

`clearok(win, true)` requests that the next `wrefresh(win)` resets the screen
buffer before drawing, effectively clearing the terminal.

## Copying windows

`overwrite(src, dst)` copies the entire contents of one window to another.
`overlay(src, dst)` performs the same operation but skips space characters
from the source so the destination shows through. Both functions operate
on the windows' backing buffers and do not immediately redraw the screen.

## Scrolling

Use `wscrl(win, lines)` to scroll a window explicitly. A positive `lines`
value scrolls the region up while a negative value scrolls it down. Newly
exposed lines are cleared using the window's current attributes.
Calling `scroll(win)` moves a window up by one line if scrolling is enabled.
The portion affected by scrolling can be restricted with
`wsetscrreg(win, top, bottom)` (or `setscrreg` for `stdscr`). The
`top` and `bottom` arguments specify the first and last lines of the
scrolling region relative to the window.

## Moving windows

`mvwin(win, y, x)` repositions a window so its upper-left corner falls within
the visible screen. Coordinates are clamped when necessary to keep the entire
window on screen.

## Resizing

`wresize(win, lines, cols)` changes the dimensions of a window. When used on a
pad, the backing buffers are reallocated. Call `resizeterm(lines, cols)` to
update `stdscr` and all windows after the terminal size changes. When this
happens `wgetch()` will return `KEY_RESIZE` so applications can react.

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
`mvwprintw` and `mvprintw` move the cursor first. Similar helpers
`mvwaddch`, `mvaddch`, `mvwaddstr` and `mvaddstr` combine movement with
character or string output.

Insertion counterparts `winsch` and `winsstr` insert text before the current
cursor position without moving it. Convenience wrappers `insch`, `mvwinsch`,
`mvinsch`, `insstr` and `mvinsstr` are also available.

```c
wprintw(win, "Count: %d", n);
printw("%s\n", msg);
```

## Inserting and deleting characters

Characters can be inserted before the cursor with `winsch` or `winsstr`.
Existing cells shift to the right and the cursor position is unchanged.  The
wrappers `insch`, `mvwinsch`, `mvinsch`, `insstr` and `mvinsstr` provide the
same behaviour for `stdscr` or with an explicit position.

To remove the character at the cursor use `wdelch(win)`.  The remainder of the
line moves left and the final cell is filled with a space.  Convenience
wrappers `delch`, `mvwdelch` and `mvdelch` behave like the insertion helpers.

## Inserting and deleting lines

`winsdelln(win, n)` shifts the lines from the cursor to the bottom of `win` by
`n` rows. A positive value inserts blank lines while a negative value deletes
lines. The helpers `insertln()` and `deleteln()` perform the same operations on
`stdscr`.

