#include "panel.h"

#include <stdlib.h>

static PANEL *top_pan = NULL;
static PANEL *bottom_pan = NULL;

PANEL *new_panel(WINDOW *win)
{
    if (!win)
        return NULL;
    PANEL *pan = calloc(1, sizeof(PANEL));
    if (!pan)
        return NULL;
    pan->win = win;
    pan->hidden = 0;
    pan->above = NULL;
    pan->below = top_pan;
    if (top_pan)
        top_pan->above = pan;
    else
        bottom_pan = pan;
    top_pan = pan;
    return pan;
}

static void unlink_panel(PANEL *pan)
{
    if (pan->above)
        pan->above->below = pan->below;
    else
        top_pan = pan->below;
    if (pan->below)
        pan->below->above = pan->above;
    else
        bottom_pan = pan->above;
    pan->above = pan->below = NULL;
}

int hide_panel(PANEL *pan)
{
    if (!pan || pan->hidden)
        return -1;
    unlink_panel(pan);
    pan->hidden = 1;
    return 0;
}

int show_panel(PANEL *pan)
{
    if (!pan)
        return -1;
    if (!pan->hidden)
        return 0;
    pan->hidden = 0;
    pan->below = top_pan;
    pan->above = NULL;
    if (top_pan)
        top_pan->above = pan;
    else
        bottom_pan = pan;
    top_pan = pan;
    return 0;
}

void update_panels(void)
{
    for (PANEL *p = bottom_pan; p; p = p->above) {
        if (!p->hidden)
            wnoutrefresh(p->win);
    }
}
