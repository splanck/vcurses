#include <check.h>
#include "../include/curses.h"

START_TEST(test_getyx_macro)
{
    WINDOW *w = newwin(3,4,1,2);
    wmove(w,2,1);
    int y=-1,x=-1;
    getyx(w,y,x);
    ck_assert_int_eq(y,2);
    ck_assert_int_eq(x,1);
    delwin(w);
}
END_TEST

START_TEST(test_getbegyx_macro)
{
    WINDOW *w = newwin(2,5,4,7);
    int y=-1,x=-1;
    getbegyx(w,y,x);
    ck_assert_int_eq(y,4);
    ck_assert_int_eq(x,7);
    delwin(w);
}
END_TEST

START_TEST(test_getmaxyx_macro)
{
    WINDOW *w = newwin(5,6,0,0);
    int y=-1,x=-1;
    getmaxyx(w,y,x);
    ck_assert_int_eq(y,5);
    ck_assert_int_eq(x,6);
    delwin(w);
}
END_TEST

START_TEST(test_getparyx_macro)
{
    WINDOW *p = newwin(10,10,3,4);
    WINDOW *s = subwin(p,4,4,5,7);
    int y=-2,x=-2;
    getparyx(s,y,x);
    ck_assert_int_eq(y,2);
    ck_assert_int_eq(x,3);
    delwin(s);
    delwin(p);
}
END_TEST

Suite *macro_suite(void)
{
    Suite *s = suite_create("macros");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_getyx_macro);
    tcase_add_test(tc, test_getbegyx_macro);
    tcase_add_test(tc, test_getmaxyx_macro);
    tcase_add_test(tc, test_getparyx_macro);
    suite_add_tcase(s, tc);
    return s;
}
