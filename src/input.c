#include "curses.h"
#include <unistd.h>
#include <stdlib.h>

static int parse_escape_sequence(void) {
    char ch;
    if (read(STDIN_FILENO, &ch, 1) != 1)
        return 27;

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
    if (read(STDIN_FILENO, &c, 1) != 1)
        return -1;

    if (win->keypad_mode && c == '\x1b')
        return parse_escape_sequence();

    return (unsigned char)c;
}

int getch(void) {
    return wgetch(stdscr);
}

int keypad(WINDOW *win, bool yes) {
    if (!win)
        return -1;
    win->keypad_mode = yes ? 1 : 0;
    return 0;
}

