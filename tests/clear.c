#include <check.h>
#include "../include/curses.h"

extern int _vc_screen_get_cell(int y, int x, char *ch, int *attr);

START_TEST(test_clearok_flag)
{
    WINDOW *w = newwin(1,1,0,0);
    ck_assert_int_eq(clearok(w, true), 0);
    ck_assert_int_eq(w->clearok, 1);
    ck_assert_int_eq(clearok(w, false), 0);
    ck_assert_int_eq(w->clearok, 0);
    delwin(w);
}
END_TEST

START_TEST(test_clearok_clears_screen)
{
    WINDOW *saved = stdscr;
    stdscr = newwin(2,2,0,0);
    waddstr(stdscr, "ab");
    wmove(stdscr,1,0);
    waddstr(stdscr, "cd");
    wrefresh(stdscr);
    clearok(stdscr, true);
    wrefresh(stdscr);
    char ch;
    for(int r=0;r<2;r++)
        for(int c=0;c<2;c++) {
            _vc_screen_get_cell(r,c,&ch,NULL);
            ck_assert_int_eq(ch,' ');
        }
    delwin(stdscr);
    stdscr = saved;
}
END_TEST

Suite *clear_suite(void)
{
    Suite *s = suite_create("clear");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_clearok_flag);
    tcase_add_test(tc, test_clearok_clears_screen);
    suite_add_tcase(s, tc);
    return s;
}
