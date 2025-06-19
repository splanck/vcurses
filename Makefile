CC ?= cc
AR ?= ar
CFLAGS ?= -Iinclude -Wall -Wextra -fPIC
BUILD := build
LIB := libvcurses.a
SRCS := src/vcurses.c src/curses.c
OBJS := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS))

all: $(LIB)

$(BUILD)/%.o: src/%.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

clean:
	rm -rf $(BUILD) $(LIB)

.PHONY: all clean
