#include <check.h>
#include "../include/curses.h"
#include <termios.h>

extern struct termios orig_termios;

START_TEST(test_nl_sets_onlcr)
{
    orig_termios.c_oflag = 0;
    nl();
    ck_assert_int_eq(orig_termios.c_oflag & ONLCR, ONLCR);
}
END_TEST

START_TEST(test_nonl_clears_onlcr)
{
    orig_termios.c_oflag = ONLCR;
    nonl();
    ck_assert_int_eq(orig_termios.c_oflag & ONLCR, 0);
}
END_TEST

Suite *term_modes_suite(void)
{
    Suite *s = suite_create("term_modes");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_nl_sets_onlcr);
    tcase_add_test(tc, test_nonl_clears_onlcr);
    suite_add_tcase(s, tc);
    return s;
}
