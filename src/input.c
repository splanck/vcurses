#include "curses.h"
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <wchar.h>
#include <stdarg.h>
#include <stdio.h>

/* mouse event queue helper */
extern void _vc_mouse_push_event(mmask_t bstate, int x, int y);
extern void _vc_mouse_flush_events(void);

/* simple push-back stack for characters */
#define INPUT_QSIZE 32
#ifdef VCURSES_WIDE
static wchar_t input_queue[INPUT_QSIZE];
#else
static int input_queue[INPUT_QSIZE];
#endif
static int input_qcount = 0;

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

static int esc_delay = 50;

static int parse_escape_sequence(WINDOW *win) {
    char ch;
    struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };
    int delay = win && win->notimeout ? 0 : esc_delay;
    if (poll(&pfd, 1, delay) <= 0)
        return 27;
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
        case 'M': return KEY_ENTER;
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

    if (input_qcount > 0) {
#ifdef VCURSES_WIDE
        int ch = (int)input_queue[--input_qcount];
#else
        int ch = input_queue[--input_qcount];
#endif
        if (win->keypad_mode) {
            if (ch == '\x7f' || ch == '\b')
                return KEY_BACKSPACE;
            if (ch == '\n' || ch == '\r')
                return KEY_ENTER;
        }
        if (ch < KEY_UP) {
            unsigned char uc = (unsigned char)ch;
            if (!win->meta_mode)
                uc &= 0x7F;
            return uc;
        }
        return ch;
    }

    char c;
    int timeout = win->delay;

    struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };
    int pr = poll(&pfd, 1, timeout);
    if (pr <= 0)
        return -1;

    if (read(STDIN_FILENO, &c, 1) != 1)
        return -1;

    if (win->keypad_mode) {
        if (c == '\x1b')
            return parse_escape_sequence(win);
        if (c == '\x7f' || c == '\b')
            return KEY_BACKSPACE;
        if (c == '\n' || c == '\r')
            return KEY_ENTER;
    }

    unsigned char uc = (unsigned char)c;
    if (!win->meta_mode)
        uc &= 0x7F;
    return uc;
}

int getch(void) {
    return wgetch(stdscr);
}

int wget_wch(WINDOW *win, wchar_t *wch) {
    int ch = wgetch(win);
    if (ch < 0)
        return -1;
    if (ch >= KEY_UP) {
        if (wch)
            *wch = ch;
        return ch;
    }
    char c = (char)ch;
#ifdef VCURSES_WIDE
    mbstate_t st = {0};
    wchar_t wc;
    if (mbrtowc(&wc, &c, 1, &st) == (size_t)-1)
        return -1;
    if (wch)
        *wch = wc;
#else
    if (wch)
        *wch = (wchar_t)c;
#endif
    return 0;
}

int get_wch(wchar_t *wch) {
    return wget_wch(stdscr, wch);
}

int ungetch(int ch) {
    if (input_qcount >= INPUT_QSIZE)
        return -1;
#ifdef VCURSES_WIDE
    input_queue[input_qcount++] = (wchar_t)ch;
#else
    input_queue[input_qcount++] = ch;
#endif
    return 0;
}

int unget_wch(wchar_t wch) {
#ifdef VCURSES_WIDE
    if (input_qcount >= INPUT_QSIZE)
        return -1;
    input_queue[input_qcount++] = wch;
    return 0;
#else
    if (wch > 0x7f)
        return -1;
    return ungetch((int)wch);
#endif
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

int wgetnstr(WINDOW *win, char *buf, int n) {
    if (!win || !buf || n <= 0)
        return -1;

    int ch;
    int count = 0;

    while (count < n && (ch = wgetch(win)) != -1) {
        if (ch == '\n' || ch == '\r') {
            buf[count] = '\0';
            return 0;
        }
        buf[count++] = (char)ch;
    }

    if (ch == -1)
        return -1;

    buf[count] = '\0';

    if (count == n) {
        while ((ch = wgetch(win)) != -1) {
            if (ch == '\n' || ch == '\r')
                break;
        }
        if (ch == -1)
            return -1;
    }

    return 0;
}

int getstr(char *buf) {
    return wgetstr(stdscr, buf);
}

int getnstr(char *buf, int n) {
    return wgetnstr(stdscr, buf, n);
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

int timeout(int delay) {
    return wtimeout(stdscr, delay);
}

int nodelay(WINDOW *win, bool bf) {
    return wtimeout(win, bf ? 0 : -1);
}

int notimeout(WINDOW *win, bool bf) {
    if (!win)
        return -1;
    win->notimeout = bf ? 1 : 0;
    return 0;
}

int meta(WINDOW *win, bool bf) {
    if (!win)
        return -1;
    win->meta_mode = bf ? 1 : 0;
    return 0;
}

int halfdelay(int tenths) {
    if (tenths <= 0 || tenths > 255 || !stdscr)
        return -1;
    if (cbreak() == -1)
        return -1;
    return wtimeout(stdscr, tenths * 100);
}

int set_escdelay(int ms) {
    if (ms < 0)
        return -1;
    esc_delay = ms;
    return 0;
}

int flushinp(void) {
    input_qcount = 0;
    _vc_mouse_flush_events();
    return 0;
}

static int vwscanw_internal(WINDOW *win, const char *fmt, va_list ap) {
    if (!win || !fmt)
        return -1;

    char buf[1024];
    if (wgetnstr(win, buf, (int)sizeof(buf) - 1) == -1)
        return -1;

    return vsscanf(buf, fmt, ap);
}

int wscanw(WINDOW *win, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vwscanw_internal(win, fmt, ap);
    va_end(ap);
    return r;
}

int scanw(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vwscanw_internal(stdscr, fmt, ap);
    va_end(ap);
    return r;
}

int mvwscanw(WINDOW *win, int y, int x, const char *fmt, ...) {
    if (wmove(win, y, x) == -1)
        return -1;
    va_list ap;
    va_start(ap, fmt);
    int r = vwscanw_internal(win, fmt, ap);
    va_end(ap);
    return r;
}

int mvscanw(int y, int x, const char *fmt, ...) {
    if (move(y, x) == -1)
        return -1;
    va_list ap;
    va_start(ap, fmt);
    int r = vwscanw_internal(stdscr, fmt, ap);
    va_end(ap);
    return r;
}

