#include <check.h>
#include "../include/curses.h"

START_TEST(test_touchwin_marks_all_lines)
{
    WINDOW *w = newwin(2,2,0,0);
    wrefresh(w); /* clear initial dirty */
    touchwin(w);
    ck_assert_int_eq(w->dirty[0], 1);
    ck_assert_int_eq(w->dirty[1], 1);
    wrefresh(w);
    ck_assert_int_eq(w->dirty[0], 0);
    ck_assert_int_eq(w->dirty[1], 0);
    delwin(w);
}
END_TEST

START_TEST(test_wtouchln_subset)
{
    WINDOW *w = newwin(3,3,0,0);
    wrefresh(w);
    wtouchln(w, 1, 1, 1);
    ck_assert_int_eq(w->dirty[0], 0);
    ck_assert_int_eq(w->dirty[1], 1);
    ck_assert_int_eq(w->dirty[2], 0);
    wrefresh(w);
    ck_assert_int_eq(w->dirty[1], 0);
    delwin(w);
}
END_TEST

Suite *touch_suite(void)
{
    Suite *s = suite_create("touch");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_touchwin_marks_all_lines);
    tcase_add_test(tc, test_wtouchln_subset);
    suite_add_tcase(s, tc);
    return s;
}

