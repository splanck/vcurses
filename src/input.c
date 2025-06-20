#include "curses.h"
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>

/* mouse event queue helper */
extern void _vc_mouse_push_event(mmask_t bstate, int x, int y);

static int read_number(int *out, char *delim)
{
    char ch;
    char digits[16];
    int i = 0;
    while (i < (int)sizeof(digits) - 1) {
        if (read(STDIN_FILENO, &ch, 1) != 1)
            return -1;
        if (ch >= '0' && ch <= '9') {
            digits[i++] = ch;
        } else {
            break;
        }
    }
    digits[i] = '\0';
    *out = atoi(digits);
    *delim = ch;
    return 0;
}

static int parse_escape_sequence(void) {
    char ch;
    if (read(STDIN_FILENO, &ch, 1) != 1)
        return 27;

    if (ch == '[') {
        if (read(STDIN_FILENO, &ch, 1) != 1)
            return 27;
        if (ch == '<') {
            int b, x, y;
            char d;
            if (read_number(&b, &d) == -1 || d != ';')
                return 27;
            if (read_number(&x, &d) == -1 || d != ';')
                return 27;
            if (read_number(&y, &d) == -1)
                return 27;
            char final = d;
            if (final != 'm' && final != 'M') {
                if (read(STDIN_FILENO, &final, 1) != 1)
                    return 27;
            }

            mmask_t bstate = 0;
            if (final == 'm') {
                switch (b) {
                case 0: bstate = BUTTON1_RELEASED; break;
                case 1: bstate = BUTTON2_RELEASED; break;
                case 2: bstate = BUTTON3_RELEASED; break;
                case 64: bstate = BUTTON4_RELEASED; break;
                case 65: bstate = BUTTON5_RELEASED; break;
                }
            } else {
                switch (b) {
                case 0: bstate = BUTTON1_PRESSED; break;
                case 1: bstate = BUTTON2_PRESSED; break;
                case 2: bstate = BUTTON3_PRESSED; break;
                case 64: bstate = BUTTON4_PRESSED; break;
                case 65: bstate = BUTTON5_PRESSED; break;
                }
            }

            if (bstate) {
                _vc_mouse_push_event(bstate, x - 1, y - 1);
                return KEY_MOUSE;
            }
            return 27;
        } else if (ch >= '0' && ch <= '9') {
            char digits[4];
            int i = 0;
            digits[i++] = ch;
            while (i < 3) {
                if (read(STDIN_FILENO, &ch, 1) != 1)
                    return 27;
                if (ch >= '0' && ch <= '9')
                    digits[i++] = ch;
                else
                    break;
            }
            digits[i] = '\0';
            if (ch == '~') {
                int code = atoi(digits);
                switch (code) {
                case 1: return KEY_HOME;
                case 2: return KEY_IC;
                case 3: return KEY_DC;
                case 4: return KEY_END;
                case 5: return KEY_PPAGE;
                case 6: return KEY_NPAGE;
                case 11: return KEY_F1;
                case 12: return KEY_F2;
                case 13: return KEY_F3;
                case 14: return KEY_F4;
                case 15: return KEY_F5;
                case 17: return KEY_F6;
                case 18: return KEY_F7;
                case 19: return KEY_F8;
                case 20: return KEY_F9;
                case 21: return KEY_F10;
                case 23: return KEY_F11;
                case 24: return KEY_F12;
                default: return 27;
                }
            } else {
                switch (ch) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
                case 'H': return KEY_HOME;
                case 'F': return KEY_END;
                default: return 27;
                }
            }
        } else {
            switch (ch) {
            case 'A': return KEY_UP;
            case 'B': return KEY_DOWN;
            case 'C': return KEY_RIGHT;
            case 'D': return KEY_LEFT;
            case 'H': return KEY_HOME;
            case 'F': return KEY_END;
            default: return 27;
            }
        }
    } else if (ch == 'O') {
        if (read(STDIN_FILENO, &ch, 1) != 1)
            return 27;
        switch (ch) {
        case 'A': return KEY_UP;
        case 'B': return KEY_DOWN;
        case 'C': return KEY_RIGHT;
        case 'D': return KEY_LEFT;
        case 'H': return KEY_HOME;
        case 'F': return KEY_END;
        case 'P': return KEY_F1;
        case 'Q': return KEY_F2;
        case 'R': return KEY_F3;
        case 'S': return KEY_F4;
        default: return 27;
        }
    }

    return 27;
}

int wgetch(WINDOW *win) {
    if (!win)
        return -1;

    char c;
    int timeout = win->delay;

    struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };
    int pr = poll(&pfd, 1, timeout);
    if (pr <= 0)
        return -1;

    if (read(STDIN_FILENO, &c, 1) != 1)
        return -1;

    if (win->keypad_mode && c == '\x1b')
        return parse_escape_sequence();

    return (unsigned char)c;
}

int getch(void) {
    return wgetch(stdscr);
}

int wgetstr(WINDOW *win, char *buf) {
    if (!win || !buf)
        return -1;

    int ch;
    char *p = buf;

    while ((ch = wgetch(win)) != -1) {
        if (ch == '\n' || ch == '\r') {
            *p = '\0';
            return 0;
        }
        *p++ = (char)ch;
    }
    return -1;
}

int getstr(char *buf) {
    return wgetstr(stdscr, buf);
}

int keypad(WINDOW *win, bool yes) {
    if (!win)
        return -1;
    win->keypad_mode = yes ? 1 : 0;
    return 0;
}

int wtimeout(WINDOW *win, int delay) {
    if (!win)
        return -1;
    win->delay = delay;
    return 0;
}

int nodelay(WINDOW *win, bool bf) {
    return wtimeout(win, bf ? 0 : -1);
}

int halfdelay(int tenths) {
    if (tenths <= 0 || tenths > 255 || !stdscr)
        return -1;
    if (cbreak() == -1)
        return -1;
    return wtimeout(stdscr, tenths * 100);
}

