#include <check.h>
#include "../include/curses.h"

Suite *scroll_suite(void);
Suite *input_suite(void);
Suite *color_suite(void);
Suite *pad_suite(void);
Suite *macro_suite(void);
Suite *copy_suite(void);
Suite *attr_suite(void);
Suite *erase_suite(void);
Suite *touch_suite(void);
Suite *clear_suite(void);
Suite *term_modes_suite(void);
Suite *has_key_suite(void);

START_TEST(test_newwin_basic)
{
    WINDOW *w = newwin(5, 10, 2, 3);
    ck_assert_ptr_nonnull(w);
    ck_assert_int_eq(w->maxy, 5);
    ck_assert_int_eq(w->maxx, 10);
    ck_assert_int_eq(w->begy, 2);
    ck_assert_int_eq(w->begx, 3);
    ck_assert_int_eq(w->cury, 0);
    ck_assert_int_eq(w->curx, 0);
    ck_assert_ptr_null(w->parent);
    ck_assert_int_eq(w->keypad_mode, 0);
    ck_assert_int_eq(w->attr, COLOR_PAIR(0));
    delwin(w);
}
END_TEST

START_TEST(test_wmove_valid)
{
    WINDOW *w = newwin(3, 4, 0, 0);
    int r = wmove(w, 2, 1);
    ck_assert_int_eq(r, 0);
    ck_assert_int_eq(w->cury, 2);
    ck_assert_int_eq(w->curx, 1);
    delwin(w);
}
END_TEST

START_TEST(test_wmove_invalid)
{
    WINDOW *w = newwin(3, 4, 0, 0);
    ck_assert_int_eq(wmove(w, 3, 0), -1);
    ck_assert_int_eq(wmove(NULL, 0, 0), -1);
    delwin(w);
}
END_TEST

START_TEST(test_waddstr_updates_cursor)
{
    WINDOW *w = newwin(5, 10, 0, 0);
    ck_assert_int_eq(waddstr(w, "hello"), 0);
    ck_assert_int_eq(w->curx, 5);
    ck_assert_int_eq(w->cury, 0);
    delwin(w);
}
END_TEST

START_TEST(test_waddstr_null)
{
    WINDOW *w = newwin(5, 5, 0, 0);
    ck_assert_int_eq(waddstr(w, NULL), -1);
    ck_assert_int_eq(waddstr(NULL, "hi"), -1);
    delwin(w);
}
END_TEST

START_TEST(test_waddch_updates_cursor)
{
    WINDOW *w = newwin(2, 2, 0, 0);
    ck_assert_int_eq(waddch(w, 'x'), 0);
    ck_assert_int_eq(w->curx, 1);
    delwin(w);
}
END_TEST

START_TEST(test_move_and_wrappers)
{
    WINDOW *saved = stdscr;
    stdscr = newwin(3, 3, 0, 0);
    ck_assert_int_eq(move(1, 1), 0);
    ck_assert_int_eq(stdscr->cury, 1);
    ck_assert_int_eq(addch('a'), 0);
    ck_assert_int_eq(stdscr->curx, 2);
    ck_assert_int_eq(addstr("bc"), 0);
    ck_assert_int_eq(stdscr->curx, 4);
    delwin(stdscr);
    stdscr = saved;
}
END_TEST

START_TEST(test_mvwaddch_sets_position)
{
    WINDOW *w = newwin(2, 3, 0, 0);
    ck_assert_int_eq(mvwaddch(w, 1, 1, 'z'), 0);
    ck_assert_int_eq(w->cury, 1);
    ck_assert_int_eq(w->curx, 2);
    delwin(w);
}
END_TEST

START_TEST(test_mvaddstr_wrapper)
{
    WINDOW *saved = stdscr;
    stdscr = newwin(2, 5, 0, 0);
    ck_assert_int_eq(mvaddstr(0, 1, "hi"), 0);
    ck_assert_int_eq(stdscr->cury, 0);
    ck_assert_int_eq(stdscr->curx, 3);
    delwin(stdscr);
    stdscr = saved;
}
END_TEST

START_TEST(test_mvwprintw_basic)
{
    WINDOW *w = newwin(2, 6, 0, 0);
    ck_assert_int_eq(mvwprintw(w, 1, 2, "%s", "a"), 0);
    ck_assert_int_eq(w->cury, 1);
    ck_assert_int_eq(w->curx, 3);
    delwin(w);
}
END_TEST

START_TEST(test_mvprintw_wrapper)
{
    WINDOW *saved = stdscr;
    stdscr = newwin(2, 6, 0, 0);
    ck_assert_int_eq(mvprintw(1, 2, "%d", 5), 0);
    ck_assert_int_eq(stdscr->cury, 1);
    ck_assert_int_eq(stdscr->curx, 3);
    delwin(stdscr);
    stdscr = saved;
}
END_TEST

START_TEST(test_subwin_parent)
{
    WINDOW *p = newwin(10, 20, 0, 0);
    WINDOW *s = subwin(p, 5, 10, 2, 3);
    ck_assert_ptr_nonnull(s);
    ck_assert_ptr_eq(s->parent, p);
    ck_assert_int_eq(s->begy, 2);
    ck_assert_int_eq(s->begx, 3);
    ck_assert_int_eq(s->maxy, 5);
    ck_assert_int_eq(s->maxx, 10);
    delwin(s);
    delwin(p);
}
END_TEST

START_TEST(test_derwin_relative)
{
    WINDOW *p = newwin(10, 20, 5, 5);
    WINDOW *s = derwin(p, 3, 4, 1, 2);
    ck_assert_ptr_nonnull(s);
    ck_assert_ptr_eq(s->parent, p);
    ck_assert_int_eq(s->begy, 6);
    ck_assert_int_eq(s->begx, 7);
    ck_assert_int_eq(s->maxy, 3);
    ck_assert_int_eq(s->maxx, 4);
    delwin(s);
    delwin(p);
}
END_TEST

START_TEST(test_wcolor_set_pair)
{
    WINDOW *w = newwin(1, 1, 0, 0);
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLUE);
    ck_assert_int_eq(wcolor_set(w, 1, NULL), 0);
    ck_assert_int_eq(PAIR_NUMBER(w->attr), 1);
    ck_assert((w->attr & A_COLOR) != 0);
    delwin(w);
}
END_TEST

START_TEST(test_wcolor_set_minus_one)
{
    WINDOW *w = newwin(1, 1, 0, 0);
    start_color();
    use_default_colors();
    ck_assert_int_eq(wcolor_set(w, -1, NULL), 0);
    ck_assert_int_eq(w->attr & A_COLOR, 0);
    delwin(w);
}
END_TEST

START_TEST(test_wrefresh_null)
{
    ck_assert_int_eq(wrefresh(NULL), -1);
}
END_TEST

START_TEST(test_wrefresh_basic)
{
    WINDOW *w = newwin(1, 3, 0, 0);
    waddstr(w, "abc");
    ck_assert_int_eq(wrefresh(w), 0);
    delwin(w);
}
END_TEST

Suite *window_suite(void)
{
    Suite *s = suite_create("windows");
    TCase *tc = tcase_create("core");

    tcase_add_test(tc, test_newwin_basic);
    tcase_add_test(tc, test_wmove_valid);
    tcase_add_test(tc, test_wmove_invalid);
    tcase_add_test(tc, test_waddstr_updates_cursor);
    tcase_add_test(tc, test_waddstr_null);
    tcase_add_test(tc, test_waddch_updates_cursor);
    tcase_add_test(tc, test_move_and_wrappers);
    tcase_add_test(tc, test_mvwaddch_sets_position);
    tcase_add_test(tc, test_mvaddstr_wrapper);
    tcase_add_test(tc, test_mvwprintw_basic);
    tcase_add_test(tc, test_mvprintw_wrapper);
    tcase_add_test(tc, test_subwin_parent);
    tcase_add_test(tc, test_derwin_relative);
    tcase_add_test(tc, test_wcolor_set_pair);
    tcase_add_test(tc, test_wcolor_set_minus_one);
    tcase_add_test(tc, test_wrefresh_null);
    tcase_add_test(tc, test_wrefresh_basic);
    suite_add_tcase(s, tc);
    return s;
}

int main(void)
{
    Suite *s1 = window_suite();
    Suite *s2 = scroll_suite();
    Suite *s3 = input_suite();
    Suite *s4 = color_suite();
    Suite *s5 = pad_suite();
    Suite *s6 = macro_suite();
    Suite *s7 = copy_suite();
    Suite *s8 = attr_suite();
    Suite *s9 = erase_suite();
    Suite *s10 = touch_suite();
    Suite *s11 = clear_suite();
    Suite *s12 = term_modes_suite();
    Suite *s13 = has_key_suite();
    SRunner *sr = srunner_create(s1);
    srunner_add_suite(sr, s2);
    srunner_add_suite(sr, s3);
    srunner_add_suite(sr, s4);
    srunner_add_suite(sr, s5);
    srunner_add_suite(sr, s6);
    srunner_add_suite(sr, s7);
    srunner_add_suite(sr, s8);
    srunner_add_suite(sr, s9);
    srunner_add_suite(sr, s10);
    srunner_add_suite(sr, s11);
    srunner_add_suite(sr, s12);
    srunner_add_suite(sr, s13);
    srunner_run_all(sr, CK_ENV); // use CK_ENV to get TAP or not
    int nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? 0 : 1;
}
