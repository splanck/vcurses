#include "vcurses.h"
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

static struct termios orig_termios;
static int term_initialized = 0;

static void restore_terminal(void) {
    if (term_initialized) {
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
        term_initialized = 0;
    }
}

static void handle_signal(int sig) {
    restore_terminal();
    signal(sig, SIG_DFL);
    raise(sig);
}

int initscr(void) {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        return -1;
    }

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) {
        return -1;
    }

    term_initialized = 1;
    atexit(restore_terminal);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGHUP, handle_signal);

    return 0;
}

int endwin(void) {
    restore_terminal();
    return 0;
}
