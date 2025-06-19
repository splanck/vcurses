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
    int attr; /* current attributes */
} WINDOW;

int vc_init(void);
WINDOW *initscr(void);
int endwin(void);
int clear(void);
int clrtobot(void);
int clrtoeol(void);
int refresh(void);

#ifdef __cplusplus
}
#endif

#endif /* VCURSES_H */
