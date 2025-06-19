#include "curses.h"
#include <unistd.h>

int main(void) {
    if (initscr() == NULL) {
        return 1;
    }

    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);

    wmove(stdscr, 0, 0);
    wcolor_set(stdscr, 1, NULL);
    waddstr(stdscr, "Red text\n");

    wmove(stdscr, 1, 0);
    wcolor_set(stdscr, 2, NULL);
    waddstr(stdscr, "Green text\n");

    refresh();
    sleep(2);

    endwin();
    return 0;
}
