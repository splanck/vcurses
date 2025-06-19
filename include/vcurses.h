#ifndef VCURSES_H
#define VCURSES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct window {
    int begy, begx; /* origin */
    int maxy, maxx; /* size */
    int cury, curx; /* cursor position */
    struct window *parent; /* parent for subwindows */
    int keypad_mode; /* keypad enabled */
    int scroll; /* scrolling enabled */
    int delay; /* input delay in ms (-1 blocking) */
    int attr; /* current attributes */
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

#ifdef __cplusplus
}
#endif

#endif /* VCURSES_H */
