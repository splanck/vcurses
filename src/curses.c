#include "curses.h"
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdio.h>

static struct termios orig_termios;
static int term_initialized = 0;
WINDOW *stdscr = NULL;

static void restore_terminal(void) {
    if (term_initialized) {
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
        term_initialized = 0;
    }
}

static void handle_signal(int sig) {
    restore_terminal();
    signal(sig, SIG_DFL);
    raise(sig);
}

static void handle_winch(int sig) {
    (void)sig;
    if (stdscr) {
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
            stdscr->maxy = ws.ws_row;
            stdscr->maxx = ws.ws_col;
        }
    }
}

WINDOW *initscr(void) {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        return NULL;
    }

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) {
        return NULL;
    }

    term_initialized = 1;
    atexit(restore_terminal);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGHUP, handle_signal);
    signal(SIGWINCH, handle_winch);

    struct winsize ws;
    int rows = 24, cols = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        rows = ws.ws_row;
        cols = ws.ws_col;
    }

    stdscr = calloc(1, sizeof(WINDOW));
    if (!stdscr) {
        return NULL;
    }
    stdscr->begy = 0;
    stdscr->begx = 0;
    stdscr->maxy = rows;
    stdscr->maxx = cols;
    stdscr->cury = 0;
    stdscr->curx = 0;
    stdscr->parent = NULL;
    stdscr->keypad_mode = 0;
    stdscr->attr = COLOR_PAIR(0);

    return stdscr;
}

int endwin(void) {
    restore_terminal();
    signal(SIGWINCH, SIG_DFL);
    free(stdscr);
    stdscr = NULL;
    return 0;
}

static int parse_escape_sequence(void) {
    char ch;
    if (read(STDIN_FILENO, &ch, 1) != 1) {
        return 27;
    }

    if (ch == '[') {
        if (read(STDIN_FILENO, &ch, 1) != 1)
            return 27;
        if (ch >= '0' && ch <= '9') {
            char digits[4];
            int i = 0;
            digits[i++] = ch;
            while (i < 3) {
                if (read(STDIN_FILENO, &ch, 1) != 1)
                    return 27;
                if (ch >= '0' && ch <= '9') {
                    digits[i++] = ch;
                } else {
                    break;
                }
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
    if (read(STDIN_FILENO, &c, 1) != 1)
        return -1;

    if (win->keypad_mode && c == '\x1b')
        return parse_escape_sequence();

    return (unsigned char)c;
}

int getch(void) {
    return wgetch(stdscr);
}

int keypad(WINDOW *win, int bf) {
    if (!win)
        return -1;
    win->keypad_mode = bf ? 1 : 0;
    return 0;
}

/* ----- Color and attribute handling ----- */

#define MAX_COLOR_PAIRS COLOR_PAIRS

typedef struct { short fg; short bg; } color_pair_t;
static color_pair_t color_pairs[MAX_COLOR_PAIRS];
static int colors_initialized = 0;

int start_color(void) {
    colors_initialized = 1;
    color_pairs[0].fg = COLOR_WHITE;
    color_pairs[0].bg = COLOR_BLACK;
    return 0;
}

int init_pair(short pair, short fg, short bg) {
    if (pair < 0 || pair >= MAX_COLOR_PAIRS)
        return -1;
    color_pairs[pair].fg = fg;
    color_pairs[pair].bg = bg;
    return 0;
}

static void apply_attr(int attr) {
    /* reset attributes */
    printf("\x1b[0m");

    if (attr & A_COLOR) {
        short pair = PAIR_NUMBER(attr);
        if (pair >= 0 && pair < MAX_COLOR_PAIRS && colors_initialized) {
            short fg = color_pairs[pair].fg;
            short bg = color_pairs[pair].bg;
            printf("\x1b[%d;%dm", 30 + fg, 40 + bg);
        }
    } else if (colors_initialized) {
        /* default colors */
        printf("\x1b[%d;%dm", 30 + color_pairs[0].fg, 40 + color_pairs[0].bg);
    }

    if (attr & A_BOLD)
        printf("\x1b[1m");
    if (attr & A_UNDERLINE)
        printf("\x1b[4m");
}

void _vcurses_apply_attr(int attr) {
    apply_attr(attr);
}

int wattron(WINDOW *win, int attrs) {
    if (!win)
        return -1;
    win->attr |= attrs;
    return 0;
}

int wattroff(WINDOW *win, int attrs) {
    if (!win)
        return -1;
    win->attr &= ~attrs;
    return 0;
}

int wattrset(WINDOW *win, int attrs) {
    if (!win)
        return -1;
    win->attr = attrs;
    return 0;
}

int attron(int attrs) { return wattron(stdscr, attrs); }
int attroff(int attrs) { return wattroff(stdscr, attrs); }
int attrset(int attrs) { return wattrset(stdscr, attrs); }

int wcolor_set(WINDOW *win, short pair, void *opts) {
    (void)opts;
    if (!win)
        return -1;
    win->attr &= ~A_COLOR;
    win->attr |= COLOR_PAIR(pair);
    return 0;
}

int color_set(short pair, void *opts) { return wcolor_set(stdscr, pair, opts); }
