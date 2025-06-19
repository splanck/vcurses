#include <check.h>
#include "../include/curses.h"
#include <sys/time.h>

static long diff_ms(struct timeval *a, struct timeval *b) {
    return (b->tv_sec - a->tv_sec) * 1000L + (b->tv_usec - a->tv_usec)/1000L;
}

START_TEST(test_nodelay_returns_err)
{
    WINDOW *w = newwin(1,1,0,0);
    ck_assert_int_eq(nodelay(w, true), 0);
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
    int r = wgetch(w);
    gettimeofday(&t2, NULL);
    ck_assert_int_eq(r, -1);
    ck_assert(diff_ms(&t1, &t2) < 200);
    delwin(w);
}
END_TEST

START_TEST(test_wtimeout_delay)
{
    WINDOW *w = newwin(1,1,0,0);
    ck_assert_int_eq(wtimeout(w, 100), 0);
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
    int r = wgetch(w);
    gettimeofday(&t2, NULL);
    ck_assert_int_eq(r, -1);
    ck_assert(diff_ms(&t1, &t2) >= 100);
    delwin(w);
}
END_TEST

Suite *input_suite(void)
{
    Suite *s = suite_create("input");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_nodelay_returns_err);
    tcase_add_test(tc, test_wtimeout_delay);
    suite_add_tcase(s, tc);
    return s;
}
