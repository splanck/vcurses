#ifndef VCURSES_H
#define VCURSES_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef VCURSES_WIDE
#include <wchar.h>
typedef wchar_t vcchar_t;
#else
typedef char vcchar_t;
#endif

typedef struct window {
    int begy, begx; /* origin */
    int maxy, maxx; /* size */
    int cury, curx; /* cursor position */
    struct window *parent; /* parent for subwindows */
    int keypad_mode; /* keypad enabled */
    int scroll; /* scrolling enabled */
    int top_margin; /* top scrolling margin */
    int bottom_margin; /* bottom scrolling margin */
    int clearok; /* full screen clear requested */
    int delay; /* input delay in ms (-1 blocking) */
    int notimeout; /* disable ESC delay */
    int leaveok; /* don't reposition cursor on refresh */
    int attr; /* current attributes */
    int bkgd; /* background attributes */
    int is_pad; /* is this a pad */
    int pad_y, pad_x; /* offset into root pad */
    vcchar_t **pad_buf; /* backing buffer for pads */
    int **pad_attr; /* attribute buffer for pads */
    unsigned char *dirty; /* per-line refresh flags */
} WINDOW;

/* Color definitions */
#define COLOR_BLACK   0
#define COLOR_RED     1
#define COLOR_GREEN   2
#define COLOR_YELLOW  3
#define COLOR_BLUE    4
#define COLOR_MAGENTA 5
#define COLOR_CYAN    6
#define COLOR_WHITE   7

#define COLOR_PAIRS   256
#define COLOR_PAIR(n)   ((n) << 8)
#define PAIR_NUMBER(a)  (((a) >> 8) & 0xFF)

/* Attribute masks */
#define A_NORMAL      0x0000
#define A_COLOR       0xFF00
#define A_BOLD        0x010000
#define A_UNDERLINE   0x020000
#define A_REVERSE     0x040000
#define A_BLINK       0x080000
#define A_DIM         0x100000
#define A_STANDOUT    0x200000

int vc_init(void);
WINDOW *initscr(void);
int endwin(void);
int clear(void);
int clrtobot(void);
int clrtoeol(void);
int refresh(void);
int echo(void);
int noecho(void);
int cbreak(void);
int nocbreak(void);
int raw(void);
int noraw(void);

#ifdef __cplusplus
}
#endif

#endif /* VCURSES_H */
