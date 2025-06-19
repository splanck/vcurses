#include "curses.h"
#include <unistd.h>

int main(void) {
    if (initscr() == NULL) {
        return 1;
    }

    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);

    wmove(stdscr, 0, 0);
    wattron(stdscr, A_BOLD);
    waddstr(stdscr, "Bold text\n");
    wattroff(stdscr, A_BOLD);

    wmove(stdscr, 1, 0);
    wattron(stdscr, A_UNDERLINE);
    waddstr(stdscr, "Underlined text\n");
    wattroff(stdscr, A_UNDERLINE);

    wmove(stdscr, 2, 0);
    wattron(stdscr, A_BOLD | A_UNDERLINE);
    wcolor_set(stdscr, 1, NULL);
    waddstr(stdscr, "Bold, Underlined & Yellow\n");
    wattroff(stdscr, A_BOLD | A_UNDERLINE);
    wcolor_set(stdscr, 0, NULL);

    refresh();
    sleep(2);

    endwin();
    return 0;
}
