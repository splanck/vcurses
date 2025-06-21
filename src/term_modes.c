#include "curses.h"
#include <termios.h>
#include <unistd.h>

/* access the current termios settings stored in init.c */
extern struct termios orig_termios;

int echo(void) {
    orig_termios.c_lflag |= ECHO;
    return tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

int noecho(void) {
    orig_termios.c_lflag &= ~ECHO;
    return tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

int cbreak(void) {
    orig_termios.c_lflag &= ~ICANON;
    orig_termios.c_cc[VMIN] = 1;
    orig_termios.c_cc[VTIME] = 0;
    return tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

int nocbreak(void) {
    orig_termios.c_lflag |= ICANON;
    return tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

int raw(void) {
    orig_termios.c_lflag &= ~(ICANON | ISIG);
    orig_termios.c_cc[VMIN] = 1;
    orig_termios.c_cc[VTIME] = 0;
    return tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

int noraw(void) {
    orig_termios.c_lflag |= ICANON | ISIG;
    return tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

int nl(void) {
    orig_termios.c_oflag |= ONLCR;
    return tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

int nonl(void) {
    orig_termios.c_oflag &= ~ONLCR;
    return tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}
