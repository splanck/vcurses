# vcurses

Minimalist modern curses compatible text library.

## Building

Ensure you have a C compiler (e.g. `cc`) and `make` installed. To build the
static library run:

```sh
make
```

This creates `libvcurses.a` in the project root. You can clean the build
artifacts with:

```sh
make clean
```

## Documentation

Man pages for the core API are available in `docs/man`. Useful entries
include [initscr.3](docs/man/initscr.3) and [getch.3](docs/man/getch.3).
