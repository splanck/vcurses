#include <check.h>
#include "../include/curses.h"

extern int _vc_screen_get_cell(int y, int x, char *ch, int *attr);

START_TEST(test_wechochar_basic)
{
    WINDOW *w = newwin(1,1,0,0);
    ck_assert_int_eq(wechochar(w, 'z'), 0);
    char ch;
    _vc_screen_get_cell(0,0,&ch,NULL);
    ck_assert_int_eq(ch, 'z');
    ck_assert_int_eq(w->curx, 1);
    delwin(w);
}
END_TEST

START_TEST(test_echochar_wrapper)
{
    WINDOW *saved = stdscr;
    stdscr = newwin(1,1,0,0);
    ck_assert_int_eq(echochar('q'), 0);
    char ch;
    _vc_screen_get_cell(0,0,&ch,NULL);
    ck_assert_int_eq(ch, 'q');
    ck_assert_int_eq(stdscr->curx, 1);
    delwin(stdscr);
    stdscr = saved;
}
END_TEST

Suite *echochar_suite(void)
{
    Suite *s = suite_create("echochar");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_wechochar_basic);
    tcase_add_test(tc, test_echochar_wrapper);
    suite_add_tcase(s, tc);
    return s;
}
