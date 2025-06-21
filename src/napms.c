#include "curses.h"
#include <unistd.h>

int napms(int ms) {
    if (ms < 0)
        return -1;
    return usleep((useconds_t)ms * 1000);
}
