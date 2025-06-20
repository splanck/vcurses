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

Suite *attr_suite(void)
{
    Suite *s = suite_create("attr");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_reverse_attribute_written);
    suite_add_tcase(s, tc);
    return s;
}
