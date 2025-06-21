#ifndef PANEL_H
#define PANEL_H

#include "curses.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct panel {
    WINDOW *win;
    struct panel *above;
    struct panel *below;
    int hidden;
} PANEL;

PANEL *new_panel(WINDOW *win);
int hide_panel(PANEL *pan);
int show_panel(PANEL *pan);
void update_panels(void);

#ifdef __cplusplus
}
#endif

#endif /* PANEL_H */
