#include "curses.h"

typedef struct { short fg; short bg; } color_pair_t;
typedef struct { short r; short g; short b; } color_rgb_t;

color_pair_t _vc_color_pairs[COLOR_PAIRS];
int _vc_colors_initialized = 0;
static int _vc_use_default_colors = 0;
static color_rgb_t _vc_colors[8];

int start_color(void) {
    _vc_colors_initialized = 1;
    _vc_color_pairs[0].fg = COLOR_WHITE;
    _vc_color_pairs[0].bg = COLOR_BLACK;
    _vc_colors[COLOR_BLACK]   = (color_rgb_t){0, 0, 0};
    _vc_colors[COLOR_RED]     = (color_rgb_t){1000, 0, 0};
    _vc_colors[COLOR_GREEN]   = (color_rgb_t){0, 1000, 0};
    _vc_colors[COLOR_YELLOW]  = (color_rgb_t){1000, 1000, 0};
    _vc_colors[COLOR_BLUE]    = (color_rgb_t){0, 0, 1000};
    _vc_colors[COLOR_MAGENTA] = (color_rgb_t){1000, 0, 1000};
    _vc_colors[COLOR_CYAN]    = (color_rgb_t){0, 1000, 1000};
    _vc_colors[COLOR_WHITE]   = (color_rgb_t){1000, 1000, 1000};
    return 0;
}

int use_default_colors(void) {
    _vc_use_default_colors = 1;
    return 0;
}

int init_pair(short pair, short fg, short bg) {
    if (pair < 0 || pair >= COLOR_PAIRS)
        return -1;
    if (!_vc_use_default_colors && (fg < 0 || bg < 0))
        return -1;
    if (fg < -1 || fg > COLOR_WHITE || bg < -1 || bg > COLOR_WHITE)
        return -1;
    _vc_color_pairs[pair].fg = fg;
    _vc_color_pairs[pair].bg = bg;
    return 0;
}

int pair_content(short pair, short *fg, short *bg) {
    if (!_vc_colors_initialized || pair < 0 || pair >= COLOR_PAIRS)
        return -1;
    if (fg)
        *fg = _vc_color_pairs[pair].fg;
    if (bg)
        *bg = _vc_color_pairs[pair].bg;
    return 0;
}

int color_content(short color, short *r, short *g, short *b) {
    if (!_vc_colors_initialized || color < 0 || color > COLOR_WHITE)
        return -1;
    if (r)
        *r = _vc_colors[color].r;
    if (g)
        *g = _vc_colors[color].g;
    if (b)
        *b = _vc_colors[color].b;
    return 0;
}

int init_color(short color, short r, short g, short b) {
    if (!_vc_colors_initialized || color < 0 || color > COLOR_WHITE)
        return -1;
    _vc_colors[color] = (color_rgb_t){r, g, b};
    return 0;
}

int has_colors(void) {
    return _vc_colors_initialized;
}

int can_change_color(void) {
    return 1;
}
