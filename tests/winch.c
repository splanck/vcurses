#include <check.h>
#include "../include/curses.h"

START_TEST(test_winch_window)
{
    WINDOW *w = newwin(1,3,0,0);
    waddstr(w, "abc");
    wmove(w,0,1);
    int ch = winch(w);
    ck_assert_int_eq(ch, w->attr | 'b');
    delwin(w);
}
END_TEST

START_TEST(test_winch_pad)
{
    WINDOW *p = newpad(1,2);
    waddstr(p, "xy");
    wmove(p,0,0);
    int ch = winch(p);
    ck_assert_int_eq(ch, p->attr | 'x');
    wmove(p,0,1);
    ch = winch(p);
    ck_assert_int_eq(ch, p->attr | 'y');
    delwin(p);
}
END_TEST

START_TEST(test_mvinch_wrapper)
{
    WINDOW *saved = stdscr;
    stdscr = newwin(1,2,0,0);
    waddstr(stdscr, "hi");
    int ch = mvinch(0,1);
    ck_assert_int_eq(ch, stdscr->attr | 'i');
    delwin(stdscr);
    stdscr = saved;
}
END_TEST

START_TEST(test_mvwinch_wrapper)
{
    WINDOW *w = newwin(1,2,0,0);
    waddstr(w, "ok");
    int ch = mvwinch(w,0,1);
    ck_assert_int_eq(ch, w->attr | 'k');
    delwin(w);
}
END_TEST

Suite *winch_suite(void)
{
    Suite *s = suite_create("winch");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_winch_window);
    tcase_add_test(tc, test_winch_pad);
    tcase_add_test(tc, test_mvinch_wrapper);
    tcase_add_test(tc, test_mvwinch_wrapper);
    suite_add_tcase(s, tc);
    return s;
}
