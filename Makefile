	CC ?= cc
AR ?= ar
CFLAGS ?= -Iinclude -Wall -Wextra -fPIC
BUILD := build
LIB := libvcurses.a
SRCS := src/vcurses.c src/init.c src/curses.c src/window.c src/screen.c
OBJS := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS))

# Unit test configuration
CHECK_CFLAGS := $(shell pkg-config --cflags check 2>/dev/null)
CHECK_LIBS := $(shell pkg-config --libs check 2>/dev/null)
TEST_SRCS := $(wildcard tests/*.c)
TEST_OBJS := $(patsubst tests/%.c,$(BUILD)/tests/%.o,$(TEST_SRCS))
TEST_BIN := $(BUILD)/tests/test_suite

all: $(LIB)

$(BUILD)/%.o: src/%.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

# --- Tests ---
$(BUILD)/tests/%.o: tests/%.c $(LIB)
	@mkdir -p $(BUILD)/tests
	$(CC) $(CFLAGS) $(CHECK_CFLAGS) -c $< -o $@

$(TEST_BIN): $(LIB) $(TEST_OBJS)
	$(CC) $(TEST_OBJS) $(LIB) $(CHECK_LIBS) -o $@

test: $(TEST_BIN)
	CK_FORK=no $(TEST_BIN)

clean:
	rm -rf $(BUILD) $(LIB)

.PHONY: all clean test
