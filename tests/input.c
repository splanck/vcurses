#include <check.h>
#include "../include/curses.h"
#include <sys/time.h>
#include <signal.h>

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
    suite_add_tcase(s, tc);
    return s;
}
