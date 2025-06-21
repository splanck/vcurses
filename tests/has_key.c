#include <check.h>
#include "../include/curses.h"

START_TEST(test_has_key_recognizes_defined)
{
    ck_assert_int_ne(has_key(KEY_UP), 0);
    ck_assert_int_ne(has_key(KEY_F5), 0);
}
END_TEST

START_TEST(test_has_key_returns_zero_for_normal_chars)
{
    ck_assert_int_eq(has_key('a'), 0);
    ck_assert_int_eq(has_key(0x30), 0);
}
END_TEST

Suite *has_key_suite(void)
{
    Suite *s = suite_create("has_key");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_has_key_recognizes_defined);
    tcase_add_test(tc, test_has_key_returns_zero_for_normal_chars);
    suite_add_tcase(s, tc);
    return s;
}
