# vcurses Documentation

## Reading strings with getstr

`getstr` and `wgetstr` read characters until a newline is encountered. The
newline is not stored in the buffer and a null terminator is appended.

```c
int getstr(char *buf);
int wgetstr(WINDOW *win, char *buf);
```

Allocate a buffer large enough for the expected input. A size of 64 or 128 bytes
is typical for command prompts.

`getstr` reads from `stdscr`, while `wgetstr` accepts the window to read from.
Both respect keypad mode, returning special key codes if keypad is enabled.

## Pushing input with ungetch

Characters may be placed back onto the input stream using `ungetch`.
The next call to `getch` or `wgetch` will return the pushed value
instead of reading from the terminal.


## Formatted output

Use `wprintw(win, fmt, ...)` to print formatted text directly into a window.
`printw()` performs the same operation on `stdscr`.

```c
wprintw(win, "Value %d", i);
printw("Hello %s", name);
```

## Color customization

Call `start_color()` to enable color handling. The RGB components of the
basic colors may then be changed with:

```c
int init_color(short color, short r, short g, short b);
```

Each component ranges from 0 to 1000. Use `color_content(color, &r, &g, &b)`
to read back the stored values. `has_colors()` reports whether color mode is
active and `can_change_color()` always returns true in this implementation.

## Mouse events

Enable mouse reporting by calling `mousemask()` with the desired button masks.
When keypad mode is active, `wgetch()` returns `KEY_MOUSE` whenever a supported
mouse event occurs. Retrieve the event details with `getmouse()`:

```c
MEVENT me;
mousemask(BUTTON1_PRESSED | BUTTON1_RELEASED, NULL);

int ch = wgetch(stdscr);
if (ch == KEY_MOUSE && getmouse(&me) == 0) {
    /* me.x and me.y are zero-based coordinates */
    handle_click(me.bstate, me.x, me.y);
}
```

## Pads

Pads are off-screen windows with their own backing buffers. Create one with
`newpad(rows, cols)` and write to it using the normal output functions. Use
`subpad` to create a view into an existing pad that shares the same buffer.

Unlike regular windows which draw directly to the terminal, pads store their
contents in memory. The `prefresh` function copies a region from a pad to the
screen:

```c
int prefresh(WINDOW *pad, int pminrow, int pmincol,
             int sminrow, int smincol, int smaxrow, int smaxcol);
```

`pminrow` and `pmincol` select the upper-left corner of the pad region while
`sminrow`, `smincol`, `smaxrow` and `smaxcol` specify the destination
rectangle on screen.

## Scrolling windows

The `wscrl` function scrolls a window by a number of lines. Positive values
move the contents up, and negative values scroll down:

```c
WINDOW *log = newwin(10, 40, 0, 0);
scrollok(log, true);
wscrl(log, -1);   /* scroll down one line */
```

## Drawing lines

Use `whline()` and `wvline()` to draw repeated characters from the current
cursor position. Wrappers `hline()` and `vline()` operate on `stdscr`:

```c
whline(win, '-', 10); /* horizontal line */
wvline(win, '|', 5);  /* vertical line */
```

## Refreshing windows

Call `refresh()` to write the full screen to the terminal. When only a
portion of the display has changed, `wrefresh(win)` flushes just that
window's area.

## Clearing window regions

Three helpers manipulate the contents of a window without immediately
redrawing:

```c
int wclear(WINDOW *win);      /* blank the entire window */
int wclrtoeol(WINDOW *win);   /* clear to end of current line */
int wclrtobot(WINDOW *win);   /* clear to bottom of window */
```

Only the specified window's backing buffers are modified. The changes become
visible after calling `wrefresh()` or `prefresh()` as appropriate. Clearing
operations honour the window's current attributes when filling spaces.

## Copying windows

Use `overwrite(src, dst)` to duplicate the contents of one window into
another. `overlay(src, dst)` is similar but leaves destination characters
unchanged wherever the source contains spaces. Both functions modify only
the backing buffers; call `wrefresh()` or `prefresh()` to display the result.

## Moving windows

Use `mvwin(win, y, x)` to reposition a window. The coordinates are
automatically adjusted so the entire window remains visible on screen.

## Resizing windows

Resize an existing window with `wresize(win, rows, cols)`. For pads this will
allocate new backing buffers. When the terminal itself changes dimensions,
call `resizeterm(rows, cols)` so `stdscr` and all windows update accordingly.
Whenever a resize occurs, the library places `KEY_RESIZE` onto the input
queue. Applications that call `wgetch()` after the event will receive this
code and can redraw their layout as needed.

## Window coordinate macros

`vcurses` defines a set of macros that mirror those found in ncurses to
inspect window geometry without function calls.

```c
getyx(win, y, x);      /* y = win->cury, x = win->curx */
getbegyx(win, y, x);   /* origin of the window */
getmaxyx(win, y, x);   /* size of the window */
getparyx(win, y, x);   /* offsets relative to the parent or -1 */

int cy = getcury(win); /* single-value forms */
int bx = getbegx(win);
```

`getpary` and `getparx` compute the position of a subwindow relative to
its parent and evaluate to `-1` if the window has no parent.  These
macros operate purely on the `WINDOW` structure and therefore behave
compatibly with applications written for ncurses.

