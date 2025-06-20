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

