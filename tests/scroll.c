#include <check.h>
#include "../include/curses.h"

START_TEST(test_scrollok_flag)
{
    WINDOW *w = newwin(1,1,0,0);
    ck_assert_int_eq(scrollok(w, true), 0);
    ck_assert_int_eq(w->scroll, 1);
    ck_assert_int_eq(scrollok(w, false), 0);
    ck_assert_int_eq(w->scroll, 0);
    delwin(w);
}
END_TEST

START_TEST(test_wscrl_null)
{
    ck_assert_int_eq(wscrl(NULL,1), -1);
}
END_TEST

START_TEST(test_wscrl_basic)
{
    WINDOW *w = newwin(2,2,0,0);
    ck_assert_int_eq(wscrl(w,1), 0);
    delwin(w);
}
END_TEST

START_TEST(test_wscrl_downward)
{
    WINDOW *w = newwin(2,2,0,0);
    ck_assert_int_eq(wscrl(w,-1), 0);
    delwin(w);
}
END_TEST

START_TEST(test_scroll_requires_flag)
{
    WINDOW *w = newwin(2,1,0,0);
    waddch(w, 'a');
    waddch(w, 'b');
    ck_assert_int_eq(scroll(w), -1);
    delwin(w);
}
END_TEST

extern int _vc_screen_get_cell(int y, int x, char *ch, int *attr);

START_TEST(test_scroll_moves_content)
{
    WINDOW *saved = stdscr;
    stdscr = newwin(2,1,0,0);
    scrollok(stdscr, true);
    waddch(stdscr, 'x');
    wmove(stdscr,1,0);
    waddch(stdscr, 'y');
    ck_assert_int_eq(scroll(stdscr), 0);
    char ch;
    _vc_screen_get_cell(0,0,&ch,NULL);
    ck_assert_int_eq(ch, 'y');
    _vc_screen_get_cell(1,0,&ch,NULL);
    ck_assert_int_eq(ch, ' ');
    delwin(stdscr);
    stdscr = saved;
}
END_TEST

Suite *scroll_suite(void)
{
    Suite *s = suite_create("scroll");
    TCase *tc = tcase_create("core");

    tcase_add_test(tc, test_scrollok_flag);
    tcase_add_test(tc, test_wscrl_null);
    tcase_add_test(tc, test_wscrl_basic);
    tcase_add_test(tc, test_wscrl_downward);
    tcase_add_test(tc, test_scroll_requires_flag);
    tcase_add_test(tc, test_scroll_moves_content);
    suite_add_tcase(s, tc);
    return s;
}
