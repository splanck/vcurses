#include "curses.h"
#include <stdio.h>

int main(void) {
    if (initscr() == NULL) {
        perror("initscr");
        return 1;
    }

    keypad(stdscr, 1);

    printf("Press keys (q to quit)\n");
    refresh();

    int ch;
    while ((ch = getch()) != 'q') {
        if (ch >= KEY_F1 && ch <= KEY_F12) {
            printf("Function F%d\n", ch - KEY_F1 + 1);
        } else {
            switch (ch) {
            case KEY_UP: printf("UP\n"); break;
            case KEY_DOWN: printf("DOWN\n"); break;
            case KEY_LEFT: printf("LEFT\n"); break;
            case KEY_RIGHT: printf("RIGHT\n"); break;
            default:
                if (ch >= 32 && ch < 127)
                    printf("'%c' (%d)\n", ch, ch);
                else
                    printf("%d\n", ch);
                break;
            }
        }
        refresh();
    }

    endwin();
    return 0;
}
