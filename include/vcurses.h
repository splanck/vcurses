#ifndef VCURSES_H
#define VCURSES_H

#ifdef __cplusplus
extern "C" {
#endif

int vc_init(void);
int initscr(void);
int endwin(void);

#ifdef __cplusplus
}
#endif

#endif /* VCURSES_H */
