#include <check.h>
#include "../include/curses.h"

START_TEST(test_overwrite_basic)
{
    WINDOW *src = newpad(1,3);
    WINDOW *dst = newpad(1,3);
    waddstr(src, "a c");
    ck_assert_int_eq(overwrite(src, dst), 0);
    ck_assert_int_eq(dst->pad_buf[0][0], 'a');
    ck_assert_int_eq(dst->pad_buf[0][1], ' ');
    ck_assert_int_eq(dst->pad_buf[0][2], 'c');
    delwin(src);
    delwin(dst);
}
END_TEST

START_TEST(test_overlay_skips_spaces)
{
    WINDOW *src = newpad(1,3);
    WINDOW *dst = newpad(1,3);
    waddstr(src, "a c");
    waddstr(dst, "XYZ");
    ck_assert_int_eq(overlay(src, dst), 0);
    ck_assert_int_eq(dst->pad_buf[0][0], 'a');
    ck_assert_int_eq(dst->pad_buf[0][1], 'Y');
    ck_assert_int_eq(dst->pad_buf[0][2], 'c');
    delwin(src);
    delwin(dst);
}
END_TEST

Suite *copy_suite(void)
{
    Suite *s = suite_create("copy");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_overwrite_basic);
    tcase_add_test(tc, test_overlay_skips_spaces);
    suite_add_tcase(s, tc);
    return s;
}
