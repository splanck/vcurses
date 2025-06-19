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

Suite *scroll_suite(void)
{
    Suite *s = suite_create("scroll");
    TCase *tc = tcase_create("core");

    tcase_add_test(tc, test_scrollok_flag);
    tcase_add_test(tc, test_wscrl_null);
    tcase_add_test(tc, test_wscrl_basic);
    suite_add_tcase(s, tc);
    return s;
}
