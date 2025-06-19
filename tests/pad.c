#include <check.h>
#include "../include/curses.h"

START_TEST(test_newpad_basic)
{
    WINDOW *p = newpad(4,6);
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->maxy, 4);
    ck_assert_int_eq(p->maxx, 6);
    ck_assert_int_eq(p->is_pad, 1);
    delwin(p);
}
END_TEST

START_TEST(test_subpad_offsets)
{
    WINDOW *p = newpad(10,10);
    WINDOW *s = subpad(p, 3, 3, 2, 2);
    ck_assert_ptr_nonnull(s);
    ck_assert_ptr_eq(s->parent, p);
    ck_assert_int_eq(s->pad_y, 2);
    ck_assert_int_eq(s->pad_x, 2);
    delwin(s);
    delwin(p);
}
END_TEST

START_TEST(test_prefresh_returns_zero)
{
    WINDOW *p = newpad(5,5);
    waddstr(p, "hello");
    ck_assert_int_eq(prefresh(p,0,0,0,0,1,4), 0);
    delwin(p);
}
END_TEST

Suite *pad_suite(void)
{
    Suite *s = suite_create("pad");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_newpad_basic);
    tcase_add_test(tc, test_subpad_offsets);
    tcase_add_test(tc, test_prefresh_returns_zero);
    suite_add_tcase(s, tc);
    return s;
}
