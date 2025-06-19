#ifndef VCURSES_H
#define VCURSES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct window WINDOW;

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
