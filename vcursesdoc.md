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

