#include <check.h>
#include "../include/curses.h"

extern int _vc_screen_get_cell(int y, int x, char *ch, int *attr);

START_TEST(test_werase_clears_window)
{
    WINDOW *saved = stdscr;
    stdscr = newwin(2,2,0,0);
    WINDOW *w = newwin(1,2,0,0);
    waddstr(w, "ab");
    ck_assert_int_eq(werase(w), 0);
    char ch;
    _vc_screen_get_cell(0,0,&ch,NULL);
    ck_assert_int_eq(ch, ' ');
    _vc_screen_get_cell(0,1,&ch,NULL);
    ck_assert_int_eq(ch, ' ');
    ck_assert_int_eq(w->cury, 0);
    ck_assert_int_eq(w->curx, 0);
    delwin(w);
    delwin(stdscr);
    stdscr = saved;
}
END_TEST

START_TEST(test_erase_wrapper)
{
    WINDOW *saved = stdscr;
    stdscr = newwin(1,2,0,0);
    waddstr(stdscr, "xy");
    ck_assert_int_eq(erase(), 0);
    char ch;
    _vc_screen_get_cell(0,0,&ch,NULL);
    ck_assert_int_eq(ch, ' ');
    _vc_screen_get_cell(0,1,&ch,NULL);
    ck_assert_int_eq(ch, ' ');
    ck_assert_int_eq(stdscr->cury, 0);
    ck_assert_int_eq(stdscr->curx, 0);
    delwin(stdscr);
    stdscr = saved;
}
END_TEST

START_TEST(test_werase_uses_bkgd)
{
    WINDOW *saved = stdscr;
    stdscr = newwin(1,1,0,0);
    wbkgdset(stdscr, A_REVERSE);
    stdscr->attr = 0;
    ck_assert_int_eq(werase(stdscr), 0);
    int attr;
    _vc_screen_get_cell(0,0,NULL,&attr);
    ck_assert_int_eq(attr, A_REVERSE);
    delwin(stdscr);
    stdscr = saved;
}
END_TEST

Suite *erase_suite(void)
{
    Suite *s = suite_create("erase");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_werase_clears_window);
    tcase_add_test(tc, test_erase_wrapper);
    tcase_add_test(tc, test_werase_uses_bkgd);
    suite_add_tcase(s, tc);
    return s;
}
