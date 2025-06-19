#include "curses.h"
#include <unistd.h>
#include <stdio.h>

int main(void) {
    if (initscr() == NULL) {
        perror("initscr");
        return 1;
    }

    printf("Resize the terminal. Press Ctrl-C to exit.\n");
    refresh();

    while (1) {
        clear();
        printf("Current size: %d rows x %d cols\n", stdscr->maxy, stdscr->maxx);
        refresh();
        sleep(1);
    }

    endwin();
    return 0;
}

