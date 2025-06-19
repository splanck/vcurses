#include "curses.h"

typedef struct { short fg; short bg; } color_pair_t;

color_pair_t _vc_color_pairs[COLOR_PAIRS];
int _vc_colors_initialized = 0;

int start_color(void) {
    _vc_colors_initialized = 1;
    _vc_color_pairs[0].fg = COLOR_WHITE;
    _vc_color_pairs[0].bg = COLOR_BLACK;
    return 0;
}

int init_pair(short pair, short fg, short bg) {
    if (pair < 0 || pair >= COLOR_PAIRS)
        return -1;
    _vc_color_pairs[pair].fg = fg;
    _vc_color_pairs[pair].bg = bg;
    return 0;
}
