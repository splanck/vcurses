#include "curses.h"
#include <stdio.h>

extern int ungetch(int);

static mmask_t current_mask = 0;
static MEVENT queued_event;
static int event_pending = 0;
static int click_interval = 0;

mmask_t mousemask(mmask_t newmask, mmask_t *oldmask)
{
    mmask_t old = current_mask;
    if (oldmask)
        *oldmask = old;
    current_mask = newmask;

    if (current_mask) {
        fputs("\x1b[?1000h\x1b[?1006h", stdout);
    } else {
        fputs("\x1b[?1006l\x1b[?1000l", stdout);
    }
    fflush(stdout);
    return old;
}

void _vc_mouse_push_event(mmask_t bstate, int x, int y)
{
    if (!(current_mask & bstate))
        return;
    queued_event.id = 0;
    queued_event.x = x;
    queued_event.y = y;
    queued_event.z = 0;
    queued_event.bstate = bstate;
    event_pending = 1;
}

int getmouse(MEVENT *event)
{
    if (!event_pending)
        return -1;
    if (event)
        *event = queued_event;
    event_pending = 0;
    return 0;
}

void _vc_mouse_flush_events(void)
{
    event_pending = 0;
}

int mouseinterval(int ms)
{
    int old = click_interval;
    if (ms < 0)
        ms = 0;
    click_interval = ms;
    return old;
}

int ungetmouse(const MEVENT *event)
{
    if (!event)
        return -1;
    queued_event = *event;
    event_pending = 1;
    ungetch(KEY_MOUSE);
    return 0;
}
