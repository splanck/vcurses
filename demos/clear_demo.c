#include "curses.h"
#include <unistd.h>
#include <stdio.h>

int main(void) {
    if (initscr() == NULL) {
        perror("initscr");
        return 1;
    }

    printf("Hello, world!\n");
    refresh();
    sleep(1);

    clear();
    printf("Screen cleared!\n");
    refresh();
    sleep(1);

    endwin();
    return 0;
}
