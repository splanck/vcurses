#include <check.h>
#include "../include/curses.h"

START_TEST(test_pair_content_returns_values)
{
    start_color();
    init_pair(2, COLOR_GREEN, COLOR_BLUE);
    short fg=-1, bg=-1;
    ck_assert_int_eq(pair_content(2, &fg, &bg), 0);
    ck_assert_int_eq(fg, COLOR_GREEN);
    ck_assert_int_eq(bg, COLOR_BLUE);
}
END_TEST

START_TEST(test_color_content_defaults)
{
    start_color();
    short r=-1,g=-1,b=-1;
    ck_assert_int_eq(color_content(COLOR_RED, &r, &g, &b), 0);
    ck_assert_int_eq(r, 1000);
    ck_assert_int_eq(g, 0);
    ck_assert_int_eq(b, 0);
}
END_TEST

START_TEST(test_init_pair_minus_one)
{
    start_color();
    use_default_colors();
    ck_assert_int_eq(init_pair(3, -1, COLOR_BLUE), 0);
    short fg=-2, bg=-2;
    pair_content(3, &fg, &bg);
    ck_assert_int_eq(fg, -1);
    ck_assert_int_eq(bg, COLOR_BLUE);
}
END_TEST

Suite *color_suite(void)
{
    Suite *s = suite_create("color");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_pair_content_returns_values);
    tcase_add_test(tc, test_color_content_defaults);
    tcase_add_test(tc, test_init_pair_minus_one);
    suite_add_tcase(s, tc);
    return s;
}
