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

## Limiting input with getnstr

When a buffer has a fixed size, use `getnstr` or `wgetnstr` to cap the number
of characters read. The `n` argument specifies the maximum number of
characters stored, excluding the terminating `\0`.

```c
char buf[4];
getnstr(buf, 3); /* reads up to three characters */
```

## Pushing input with ungetch

Characters may be placed back onto the input stream using `ungetch`.
The next call to `getch` or `wgetch` will return the pushed value
instead of reading from the terminal.
Mouse events can be queued with `ungetmouse(&me)` so the next
`wgetch()` returns `KEY_MOUSE` and `getmouse()` retrieves the
provided event.
`flushinp()` clears any queued input, including mouse events.

## Key codes

When keypad mode is enabled, `wgetch()` translates certain key presses into
constants defined in `<curses.h>`.  These include:

- `KEY_UP`, `KEY_DOWN`, `KEY_LEFT`, `KEY_RIGHT`
- `KEY_HOME`, `KEY_END`, `KEY_NPAGE`, `KEY_PPAGE`
- `KEY_IC`, `KEY_DC`, `KEY_BACKSPACE`, `KEY_ENTER`
- `KEY_F1`&ndash;`KEY_F12`
- `KEY_MOUSE` and `KEY_RESIZE`

Regular printable characters are returned unchanged.
`has_key(code)` returns non-zero when `code` matches one of these constants.

### keyname lookup table

`keyname(ch)` returns a string describing a key code. Printable ASCII
characters are returned as one-character strings. Special codes resolve to
the names shown below:

| Code | String |
|------|--------|
| `KEY_UP` | "KEY_UP" |
| `KEY_DOWN` | "KEY_DOWN" |
| `KEY_LEFT` | "KEY_LEFT" |
| `KEY_RIGHT` | "KEY_RIGHT" |
| `KEY_HOME` | "KEY_HOME" |
| `KEY_END` | "KEY_END" |
| `KEY_NPAGE` | "KEY_NPAGE" |
| `KEY_PPAGE` | "KEY_PPAGE" |
| `KEY_IC` | "KEY_IC" |
| `KEY_DC` | "KEY_DC" |
| `KEY_BACKSPACE` | "KEY_BACKSPACE" |
| `KEY_ENTER` | "KEY_ENTER" |
| `KEY_F1` | "KEY_F1" |
| `KEY_F2` | "KEY_F2" |
| `KEY_F3` | "KEY_F3" |
| `KEY_F4` | "KEY_F4" |
| `KEY_F5` | "KEY_F5" |
| `KEY_F6` | "KEY_F6" |
| `KEY_F7` | "KEY_F7" |
| `KEY_F8` | "KEY_F8" |
| `KEY_F9` | "KEY_F9" |
| `KEY_F10` | "KEY_F10" |
| `KEY_F11` | "KEY_F11" |
| `KEY_F12` | "KEY_F12" |
| `KEY_MOUSE` | "KEY_MOUSE" |
| `KEY_RESIZE` | "KEY_RESIZE" |

## Input timeouts

Input waiting behaviour can be configured per window.  The helpers
below control whether `wgetch()` blocks and how it interprets escape
sequences:

```c
int nodelay(WINDOW *win, bool bf);   /* true for non-blocking */
int wtimeout(WINDOW *win, int ms);   /* delay in milliseconds */
int timeout(int ms);                  /* stdscr wrapper */
int halfdelay(int tenths);           /* sets cbreak timeout */
int notimeout(WINDOW *win, bool bf); /* disable ESC delay */
int set_escdelay(int ms);            /* ESC detection delay */
int mouseinterval(int ms);           /* click timeout */
int meta(WINDOW *win, bool bf);      /* return 8-bit input */
```

When `notimeout` is enabled, `wgetch()` does not pause after an ESC
character before returning it.  Otherwise the delay from `set_escdelay`
is used to decide whether an escape sequence is pending.  The initial
value may also be specified through the `ESCDELAY` environment
variable.

Calling `meta(win, true)` causes `wgetch()` to return 8-bit characters
unchanged. When disabled the high bit is stripped so bytes above 0x7F are
masked to 7 bits.

## Terminal modes

Functions that control how input and output are processed:

```c
int echo(void);      /* enable input echo */
int noecho(void);    /* disable input echo */
int cbreak(void);    /* disable line buffering */
int nocbreak(void);
int raw(void);       /* disable signals and line buffering */
int noraw(void);
int nl(void);        /* map \n to \r\n on output */
int nonl(void);      /* disable newline mapping */
```

`initscr()` enters raw mode with echo disabled. `nl()` enables newline
translation so each `\n` sent to the terminal becomes a carriage return
followed by a line feed. `nonl()` leaves `\n` unchanged.

## Timing functions

`napms(ms)` sleeps for `ms` milliseconds without affecting the terminal.
Input waiting can also be tuned with `wtimeout`, `halfdelay` and
`set_escdelay`, all described earlier under [Input timeouts](#input-timeouts).

## Formatted output

Use `wprintw(win, fmt, ...)` to print formatted text directly into a window.
`printw()` performs the same operation on `stdscr`.

```c
wprintw(win, "Value %d", i);
printw("Hello %s", name);
```

### Moving output

Several helpers combine cursor movement with output. They return an error if
the target position is outside the window.

```c
int mvwaddch(WINDOW *win, int y, int x, char ch);
int mvaddch(int y, int x, char ch);       /* stdscr */
int mvwaddstr(WINDOW *win, int y, int x, const char *str);
int mvaddstr(int y, int x, const char *str);
int mvwprintw(WINDOW *win, int y, int x, const char *fmt, ...);
int mvprintw(int y, int x, const char *fmt, ...);  /* stdscr */
```

### Limiting output

The `waddnstr` family writes at most `n` characters. Extra text is
discarded before reaching the window.

```c
WINDOW *w = newwin(1, 5, 0, 0);
waddnstr(w, "abcdef", 3);       /* prints "abc"    */
mvwaddnstr(w, 0, 2, "xyz", 5); /* prints "xy"     */
```

### Echoing characters

`wechochar(win, ch)` writes `ch` to `win` and immediately refreshes the
affected area. `echochar(ch)` performs the same operation on `stdscr`.

```c
wechochar(win, 'X');
echochar('Y');
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

Calling `use_default_colors()` permits `init_pair` to take `-1` as the
foreground or background.  A value of `-1` leaves that component unchanged
so the terminal's default color shows through.  When default color support is
active, `wcolor_set(win, -1, NULL)` clears any color pair on `win` while
preserving its other attributes.

## Text attributes

Attribute masks control text style:

```c
#define A_BOLD        0x010000
#define A_UNDERLINE   0x020000
#define A_REVERSE     0x040000
#define A_BLINK       0x080000
#define A_DIM         0x100000
#define A_STANDOUT    0x200000
```

Enable an attribute with `wattron(win, mask)` and disable it with
`wattroff(win, mask)`. `A_REVERSE` swaps the foreground and background
colors when output is refreshed.

## Mouse events

Enable mouse reporting by calling `mousemask()` with the desired button masks.
When keypad mode is active, `wgetch()` returns `KEY_MOUSE` whenever a supported
mouse event occurs. The interval for detecting clicks can be configured with
`mouseinterval(ms)`. Retrieve queued events with `getmouse()` or push one with
`ungetmouse()`:

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

## Panels

Panels provide simple stacking of windows. Each panel wraps a `WINDOW` and may
be shown or hidden without destroying the underlying window. Newly created or
shown panels are placed on top of the stack.

```c
PANEL *new_panel(WINDOW *win);  /* create a panel for a window */
int hide_panel(PANEL *pan);     /* remove from the stack */
int show_panel(PANEL *pan);     /* place back on top */
void update_panels(void);       /* wnoutrefresh visible panels */
```

`update_panels()` iterates from the bottom panel upward calling
`wnoutrefresh()` on each visible panel. Applications should call `doupdate()`
afterwards to write the queued regions to the terminal.

## Scrolling windows

The `wscrl` function scrolls a window by a number of lines. Positive values
move the contents up, and negative values scroll down:

```c
WINDOW *log = newwin(10, 40, 0, 0);
scrollok(log, true);
wscrl(log, -1);   /* scroll down one line */
```

Calling `scroll(win)` is equivalent to `wscrl(win, 1)` but requires that
scrolling be enabled with `scrollok(win, true)`.

Use `wsetscrreg(win, top, bottom)` to confine scrolling to a subset of
lines. The parameters define the upper and lower bounds of the region
within the window. `setscrreg` operates on `stdscr`.

## Drawing lines

Use `whline()` and `wvline()` to draw repeated characters from the current
cursor position. Wrappers `hline()` and `vline()` operate on `stdscr`:

```c
whline(win, '-', 10); /* horizontal line */
wvline(win, '|', 5);  /* vertical line */
```

### ACS line drawing characters

Mnemonic constants such as `ACS_HLINE` and `ACS_VLINE` map to the ASCII
characters used for simple boxes. They can be passed to `box`, `wborder`,
`whline` or `wvline`:

```c
box(win, ACS_VLINE, ACS_HLINE);
wborder(win, ACS_VLINE, ACS_VLINE,
        ACS_HLINE, ACS_HLINE,
        ACS_ULCORNER, ACS_URCORNER,
        ACS_LLCORNER, ACS_LRCORNER);
```

The library translates these values to printable characters so programs work
even without Unicode line drawing support.

## Inserting and deleting characters

Characters and strings can be inserted before the cursor using
`winsch(win, ch)` and `winsstr(win, str)`. Existing cells on the line shift to
the right and the last character is dropped. The cursor position itself is
unchanged. Wrappers `insch`, `mvwinsch`, `mvinsch`, `insstr` and `mvinsstr`
mirror the behaviour of the output helpers.

`wdelch(win)` removes the character under the cursor, shifting the remainder of
the line left and filling the final position with a space. Wrappers `delch`,
`mvwdelch` and `mvdelch` behave analogously.

```c
WINDOW *w = newwin(1, 8, 0, 0);
waddstr(w, "hello");
wmove(w, 0, 2);
winsch(w, 'X');   /* heXllo */
wmove(w, 0, 1);
wdelch(w);        /* hXllo */
```

Only single byte characters are currently supported. Strings longer than the
remaining space on the line are truncated.

```c
WINDOW *t = newwin(1, 4, 0, 0);
waddstr(t, "abcdef");       /* prints "abcd" */
mvwaddnstr(t, 0, 0, "123456", 3); /* prints "123" */
```

## Inserting and deleting lines

Use `winsdelln(win, n)` to shift rows starting at the cursor. Positive `n`
inserts blank lines while negative `n` removes lines. The wrappers
`insertln()` and `deleteln()` operate on `stdscr`.

## Refreshing windows

Call `refresh()` to write the full screen to the terminal. When only a
portion of the display has changed, `wrefresh(win)` flushes just that
window's area.

Sometimes external programs overwrite part of the terminal.  The
`touchwin()` and `wtouchln()` functions mark whole windows or selected lines
as dirty so the next `wrefresh()` repaints them. `redrawwin()` touches the
entire window in one call.

## Clearing window regions

Three helpers manipulate the contents of a window without immediately
redrawing:

```c
int wclear(WINDOW *win);      /* blank the entire window */
int werase(WINDOW *win);      /* clear window without full screen reset */
int erase(void);              /* stdscr wrapper for werase */
int wclrtoeol(WINDOW *win);   /* clear to end of current line */
int wclrtobot(WINDOW *win);   /* clear to bottom of window */
```

Only the specified window's backing buffers are modified. The changes become
visible after calling `wrefresh()` or `prefresh()` as appropriate. Clearing
operations honour the window's current attributes when filling spaces.
Use `werase()` (or `erase()` for `stdscr`) when you simply need to blank the
buffers without forcing a full terminal clear on the next refresh.

Calling `clearok(win, true)` marks a window so that the next `wrefresh(win)`
resets the internal screen buffer and clears the terminal before redrawing.
The flag is cleared afterward. This is helpful when an external program has
disturbed the display or you simply want to ensure a blank screen.

```c
/* restore a pristine display */
wrefresh(stdscr);
system("some-command");
clearok(stdscr, true);
wrefresh(stdscr); /* screen is cleared */
```

## Background attributes

Every window maintains a background attribute used whenever blank cells are
written by clearing or scrolling operations. Adjust it with
`wbkgdset(win, attr)` or `bkgdset(attr)` for `stdscr`. The helper
`wbkgd(win, attr)` updates the attribute and clears the window so existing
spaces adopt the new value.

```c
WINDOW *log = newwin(3, 10, 0, 0);
wbkgdset(log, COLOR_PAIR(2));
werase(log);            /* blanks use pair 2 */
scrollok(log, true);
wscrl(log, 1);          /* inserted line also uses pair 2 */
```

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

## Cursor behavior

`wrefresh(win)` normally moves the terminal cursor to the window's current
location. Calling `leaveok(win, true)` suppresses this repositioning so the
cursor stays wherever it was before the refresh. Pass `false` to restore the
default behaviour.

## Unicode handling

When the library is compiled with `-DVCURSES_WIDE` the backing buffers use
`wchar_t` instead of plain bytes. Additional functions mirror the byte oriented
API:

```c
int add_wch(wchar_t wch);           /* stdscr wrapper */
int wadd_wch(WINDOW *win, wchar_t wch);
int mvadd_wch(int y, int x, wchar_t wch);
int mvwadd_wch(WINDOW *win, int y, int x, wchar_t wch);
int get_wch(wchar_t *out);          /* stdscr wrapper */
int wget_wch(WINDOW *win, wchar_t *out);
int unget_wch(wchar_t ch);
```

Text drawn to the terminal is encoded using the current locale. Programs should
call `setlocale(LC_CTYPE, "")` early so multibyte conversion matches the
expected encoding (typically UTF-8).

## Formatted input

`wscanw(win, fmt, ...)` reads a line from `win` and parses it according to
`scanf(3)` rules. The helpers `scanw`, `mvwscanw` and `mvscanw` behave like
their output counterparts, optionally moving the cursor first or defaulting to
`stdscr`.

```c
int wscanw(WINDOW *win, const char *fmt, ...);
int scanw(const char *fmt, ...);          /* stdscr wrapper */
int mvwscanw(WINDOW *win, int y, int x, const char *fmt, ...);
int mvscanw(int y, int x, const char *fmt, ...);  /* stdscr */
```

The return value matches `vsscanf()`&mdash;the number of successfully scanned
items or `-1` on failure. Internally these functions use `wgetnstr` so input is
limited to 1023 characters.

