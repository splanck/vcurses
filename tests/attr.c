#include <check.h>
#include "../include/curses.h"

START_TEST(test_reverse_attribute_written)
{
    WINDOW *p = newpad(1, 3);
    wattron(p, A_REVERSE);
    waddstr(p, "abc");
    ck_assert_int_eq(p->pad_attr[0][0] & A_REVERSE, A_REVERSE);
    ck_assert_int_eq(p->pad_attr[0][1] & A_REVERSE, A_REVERSE);
    ck_assert_int_eq(p->pad_attr[0][2] & A_REVERSE, A_REVERSE);
    delwin(p);
}
END_TEST

START_TEST(test_blink_attribute_written)
{
    WINDOW *p = newpad(1, 1);
    wattron(p, A_BLINK);
    waddch(p, 'x');
    ck_assert_int_eq(p->pad_attr[0][0] & A_BLINK, A_BLINK);
    delwin(p);
}
END_TEST

START_TEST(test_combined_attributes_written)
{
    WINDOW *p = newpad(1, 2);
    wattron(p, A_BOLD | A_BLINK);
    waddstr(p, "ab");
    ck_assert_int_eq(p->pad_attr[0][0] & (A_BOLD|A_BLINK), A_BOLD|A_BLINK);
    ck_assert_int_eq(p->pad_attr[0][1] & (A_BOLD|A_BLINK), A_BOLD|A_BLINK);
    delwin(p);
}
END_TEST

START_TEST(test_wbkgdset_sets_value)
{
    WINDOW *w = newwin(1,1,0,0);
    ck_assert_int_eq(wbkgdset(w, A_BOLD), 0);
    ck_assert_int_eq(w->bkgd, A_BOLD);
    delwin(w);
}
END_TEST

Suite *attr_suite(void)
{
    Suite *s = suite_create("attr");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_reverse_attribute_written);
    tcase_add_test(tc, test_blink_attribute_written);
    tcase_add_test(tc, test_combined_attributes_written);
    tcase_add_test(tc, test_wbkgdset_sets_value);
    suite_add_tcase(s, tc);
    return s;
}
