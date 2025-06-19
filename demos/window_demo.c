#include "curses.h"
#include <unistd.h>
#include <stdio.h>

int main(void) {
    if (initscr() == NULL) {
        perror("initscr");
        return 1;
    }

    WINDOW *win1 = newwin(3, 20, 1, 1);
    WINDOW *win2 = newwin(3, 20, 5, 10);

    wmove(win1, 0, 0);
    waddstr(win1, "First window");
    wmove(win2, 0, 0);
    waddstr(win2, "Second window");

    refresh();
    sleep(2);

    delwin(win1);
    delwin(win2);
    endwin();
    return 0;
}
