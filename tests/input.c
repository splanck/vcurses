#include <check.h>
#include "../include/curses.h"
#include <sys/time.h>
#include <signal.h>

extern void _vc_mouse_push_event(mmask_t bstate, int x, int y);

static long diff_ms(struct timeval *a, struct timeval *b) {
    return (b->tv_sec - a->tv_sec) * 1000L + (b->tv_usec - a->tv_usec)/1000L;
}

START_TEST(test_nodelay_returns_err)
{
    WINDOW *w = newwin(1,1,0,0);
    ck_assert_int_eq(nodelay(w, true), 0);
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
    int r = wgetch(w);
    gettimeofday(&t2, NULL);
    ck_assert_int_eq(r, -1);
    ck_assert(diff_ms(&t1, &t2) < 200);
    delwin(w);
}
END_TEST

START_TEST(test_wtimeout_delay)
{
    WINDOW *w = newwin(1,1,0,0);
    ck_assert_int_eq(wtimeout(w, 100), 0);
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
    int r = wgetch(w);
    gettimeofday(&t2, NULL);
    ck_assert_int_eq(r, -1);
    ck_assert(diff_ms(&t1, &t2) >= 100);
    delwin(w);
}
END_TEST

START_TEST(test_ungetch_single)
{
    WINDOW *w = newwin(1,1,0,0);
    ck_assert_int_eq(ungetch('x'), 0);
    ck_assert_int_eq(wgetch(w), 'x');
    delwin(w);
}
END_TEST

START_TEST(test_ungetch_stack_order)
{
    WINDOW *w = newwin(1,1,0,0);
    ck_assert_int_eq(ungetch('a'), 0);
    ck_assert_int_eq(ungetch('b'), 0);
    ck_assert_int_eq(wgetch(w), 'b');
    ck_assert_int_eq(wgetch(w), 'a');
    delwin(w);
}
END_TEST

extern void _vc_resize_init(void);
extern void _vc_resize_shutdown(void);

START_TEST(test_resize_event)
{
    _vc_resize_init();
    raise(SIGWINCH);
    WINDOW *w = newwin(1,1,0,0);
    int ch = wgetch(w);
    ck_assert_int_eq(ch, KEY_RESIZE);
    delwin(w);
    _vc_resize_shutdown();
}
END_TEST

START_TEST(test_wgetnstr_limits_length)
{
    WINDOW *w = newwin(1,1,0,0);
    ungetch('\n');
    ungetch('c');
    ungetch('b');
    ungetch('a');
    char buf[3];
    ck_assert_int_eq(wgetnstr(w, buf, 2), 0);
    ck_assert_str_eq(buf, "ab");
    nodelay(w, true);
    ck_assert_int_eq(wgetch(w), -1);
    delwin(w);
}
END_TEST

START_TEST(test_getnstr_wrapper)
{
    WINDOW *saved = stdscr;
    stdscr = newwin(1,1,0,0);
    ungetch('\n');
    ungetch('y');
    ungetch('x');
    char buf[3];
    ck_assert_int_eq(getnstr(buf, 2), 0);
    ck_assert_str_eq(buf, "xy");
    nodelay(stdscr, true);
    ck_assert_int_eq(getch(), -1);
    delwin(stdscr);
    stdscr = saved;
}
END_TEST

START_TEST(test_keypad_translates_backspace)
{
    WINDOW *w = newwin(1,1,0,0);
    keypad(w, true);
    ungetch('\x7f');
    ck_assert_int_eq(wgetch(w), KEY_BACKSPACE);
    delwin(w);
}
END_TEST

START_TEST(test_keypad_translates_enter)
{
    WINDOW *w = newwin(1,1,0,0);
    keypad(w, true);
    ungetch('\n');
    ck_assert_int_eq(wgetch(w), KEY_ENTER);
    delwin(w);
}
END_TEST

START_TEST(test_meta_masks_high_bit)
{
    WINDOW *w = newwin(1,1,0,0);
    ungetch(0xC1);
    ck_assert_int_eq(wgetch(w), 0x41);
    delwin(w);
}
END_TEST

START_TEST(test_meta_returns_8bit)
{
    WINDOW *w = newwin(1,1,0,0);
    meta(w, true);
    ungetch(0xC1);
    ck_assert_int_eq(wgetch(w), 0xC1);
    delwin(w);
}
END_TEST

START_TEST(test_flushinp_clears_queue)
{
    WINDOW *w = newwin(1,1,0,0);
    ungetch('z');
    flushinp();
    nodelay(w, true);
    ck_assert_int_eq(wgetch(w), -1);
    delwin(w);
}
END_TEST

START_TEST(test_flushinp_drops_mouse)
{
    mmask_t old;
    mousemask(BUTTON1_PRESSED, &old);
    _vc_mouse_push_event(BUTTON1_PRESSED, 0, 0);
    flushinp();
    MEVENT ev;
    ck_assert_int_eq(getmouse(&ev), -1);
    mousemask(old, NULL);
}
END_TEST

START_TEST(test_ungetmouse_queues_event)
{
    MEVENT ev = { .id = 0, .x = 3, .y = 4, .z = 0, .bstate = BUTTON1_PRESSED };
    ungetmouse(&ev);
    WINDOW *w = newwin(1,1,0,0);
    ck_assert_int_eq(wgetch(w), KEY_MOUSE);
    MEVENT out;
    ck_assert_int_eq(getmouse(&out), 0);
    ck_assert_int_eq(out.x, ev.x);
    ck_assert_int_eq(out.y, ev.y);
    ck_assert_int_eq(out.bstate, ev.bstate);
    delwin(w);
}
END_TEST

START_TEST(test_mouseinterval_returns_previous)
{
    ck_assert_int_eq(mouseinterval(20), 0);
    ck_assert_int_eq(mouseinterval(5), 20);
}
END_TEST

Suite *input_suite(void)
{
    Suite *s = suite_create("input");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_nodelay_returns_err);
    tcase_add_test(tc, test_wtimeout_delay);
    tcase_add_test(tc, test_ungetch_single);
    tcase_add_test(tc, test_ungetch_stack_order);
    tcase_add_test(tc, test_resize_event);
    tcase_add_test(tc, test_wgetnstr_limits_length);
    tcase_add_test(tc, test_getnstr_wrapper);
    tcase_add_test(tc, test_keypad_translates_backspace);
    tcase_add_test(tc, test_keypad_translates_enter);
    tcase_add_test(tc, test_meta_masks_high_bit);
    tcase_add_test(tc, test_meta_returns_8bit);
    tcase_add_test(tc, test_flushinp_clears_queue);
    tcase_add_test(tc, test_flushinp_drops_mouse);
    tcase_add_test(tc, test_ungetmouse_queues_event);
    tcase_add_test(tc, test_mouseinterval_returns_previous);
    suite_add_tcase(s, tc);
    return s;
}
