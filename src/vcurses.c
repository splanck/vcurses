#include "vcurses.h"

#include <stdio.h>

/* Initialize vcurses. Currently does nothing. */
int vc_init(void) {
    return 0;
}

/* Clear the entire screen and move cursor to the home position. */
int clear(void) {
    fputs("\x1b[2J\x1b[H", stdout);
    return 0;
}

/* Clear from the cursor position to the bottom of the screen. */
int clrtobot(void) {
    fputs("\x1b[J", stdout);
    return 0;
}

/* Clear from the cursor position to the end of the current line. */
int clrtoeol(void) {
    fputs("\x1b[K", stdout);
    return 0;
}

/* Flush buffered output to the terminal. */
int refresh(void) {
    return fflush(stdout);
}
