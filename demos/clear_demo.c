#include "vcurses.h"
#include <unistd.h>
#include <stdio.h>

int main(void) {
    if (initscr() == -1) {
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
