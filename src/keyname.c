#include "curses.h"

struct keyname_entry { int code; const char *name; };

static const struct keyname_entry key_table[] = {
    {KEY_UP, "KEY_UP"},
    {KEY_DOWN, "KEY_DOWN"},
    {KEY_LEFT, "KEY_LEFT"},
    {KEY_RIGHT, "KEY_RIGHT"},
    {KEY_HOME, "KEY_HOME"},
    {KEY_END, "KEY_END"},
    {KEY_NPAGE, "KEY_NPAGE"},
    {KEY_PPAGE, "KEY_PPAGE"},
    {KEY_IC, "KEY_IC"},
    {KEY_DC, "KEY_DC"},
    {KEY_BACKSPACE, "KEY_BACKSPACE"},
    {KEY_ENTER, "KEY_ENTER"},
    {KEY_F1, "KEY_F1"},
    {KEY_F2, "KEY_F2"},
    {KEY_F3, "KEY_F3"},
    {KEY_F4, "KEY_F4"},
    {KEY_F5, "KEY_F5"},
    {KEY_F6, "KEY_F6"},
    {KEY_F7, "KEY_F7"},
    {KEY_F8, "KEY_F8"},
    {KEY_F9, "KEY_F9"},
    {KEY_F10, "KEY_F10"},
    {KEY_F11, "KEY_F11"},
    {KEY_F12, "KEY_F12"},
    {KEY_MOUSE, "KEY_MOUSE"},
    {KEY_RESIZE, "KEY_RESIZE"},
};

const char *keyname(int ch) {
    if (ch >= 32 && ch < 127) {
        static char s[2];
        s[0] = (char)ch;
        s[1] = '\0';
        return s;
    }
    for (unsigned i = 0; i < sizeof(key_table)/sizeof(key_table[0]); ++i) {
        if (key_table[i].code == ch)
            return key_table[i].name;
    }
    return NULL;
}

int has_key(int keycode) {
    for (unsigned i = 0; i < sizeof(key_table)/sizeof(key_table[0]); ++i) {
        if (key_table[i].code == keycode)
            return 1;
    }
    return 0;
}
