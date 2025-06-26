#include "curses.h"

#include <stdio.h>
#include <stdlib.h>

/* Initialize vcurses. Reads ESCDELAY from the environment. */
int vc_init(void) {
    const char *env = getenv("ESCDELAY");
    if (env) {
        char *end;
        long v = strtol(env, &end, 10);
        if (end != env && v >= 0)
            set_escdelay((int)v);
    }
    return 0;
}
