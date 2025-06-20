#include "curses.h"
#include <string.h>

static WINDOW *pad_root(WINDOW *pad)
{
    WINDOW *r = pad;
    while (r->parent && r->parent->is_pad)
        r = r->parent;
    return r;
}

extern int _vc_screen_get_cell(int y, int x, char *ch, int *attr);
extern void _vc_screen_puts(int y, int x, const char *str, int attr);

int copywin(WINDOW *src, WINDOW *dst,
            int sminrow, int smincol,
            int dminrow, int dmincol,
            int dmaxrow, int dmaxcol,
            int overlay)
{
    if (!src || !dst)
        return -1;

    int rows = dmaxrow - dminrow + 1;
    int cols = dmaxcol - dmincol + 1;
    if (rows <= 0 || cols <= 0)
        return -1;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int sr = sminrow + r;
            int sc = smincol + c;
            int dr = dminrow + r;
            int dc = dmincol + c;

            if (sr < 0 || sc < 0 || dr < 0 || dc < 0)
                continue;
            if (sr >= src->maxy || sc >= src->maxx)
                continue;
            if (dr >= dst->maxy || dc >= dst->maxx)
                continue;

            char ch = ' ';
            int attr = src->attr;

            if (src->is_pad) {
                WINDOW *sroot = pad_root(src);
                int rr = src->pad_y + sr;
                int cc = src->pad_x + sc;
                if (rr >= sroot->maxy || cc >= sroot->maxx)
                    continue;
                ch = sroot->pad_buf[rr][cc];
                attr = sroot->pad_attr[rr][cc];
            } else {
                if (_vc_screen_get_cell(src->begy + sr, src->begx + sc,
                                       &ch, &attr) == -1)
                    continue;
            }

            if (overlay && ch == ' ')
                continue;

            if (dst->is_pad) {
                WINDOW *droot = pad_root(dst);
                int rr = dst->pad_y + dr;
                int cc = dst->pad_x + dc;
                if (rr >= droot->maxy || cc >= droot->maxx)
                    continue;
                droot->pad_buf[rr][cc] = ch;
                droot->pad_attr[rr][cc] = attr;
            } else {
                char buf[2] = { ch, 0 };
                _vc_screen_puts(dst->begy + dr, dst->begx + dc, buf, attr);
            }
        }
    }

    return 0;
}

int overlay(WINDOW *src, WINDOW *dst)
{
    if (!src || !dst)
        return -1;
    int rows = src->maxy < dst->maxy ? src->maxy : dst->maxy;
    int cols = src->maxx < dst->maxx ? src->maxx : dst->maxx;
    return copywin(src, dst, 0, 0, 0, 0, rows - 1, cols - 1, 1);
}

int overwrite(WINDOW *src, WINDOW *dst)
{
    if (!src || !dst)
        return -1;
    int rows = src->maxy < dst->maxy ? src->maxy : dst->maxy;
    int cols = src->maxx < dst->maxx ? src->maxx : dst->maxx;
    return copywin(src, dst, 0, 0, 0, 0, rows - 1, cols - 1, 0);
}
