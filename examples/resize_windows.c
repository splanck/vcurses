#include "curses.h"
#include <unistd.h>
#include <stdio.h>

int main(void) {
    if (initscr() == NULL)
        return 1;

    WINDOW *top = newwin(LINES / 2 - 1, COLS - 2, 1, 1);
    WINDOW *bottom = newwin(LINES / 2 - 2, COLS - 2, LINES / 2 + 1, 1);

    while (1) {
        clear();
        wmove(top, 0, 0);
        waddstr(top, "Top window");
        wmove(bottom, 0, 0);
        waddstr(bottom, "Bottom window");

        wmove(stdscr, LINES - 1, 0);
        char buf[64];
        snprintf(buf, sizeof(buf), "Screen: %dx%d", LINES, COLS);
        waddstr(stdscr, buf);
        refresh();
        sleep(1);
    }

    delwin(top);
    delwin(bottom);
    endwin();
    return 0;
}
