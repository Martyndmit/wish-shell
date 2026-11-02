# wish — a simple Unix shell (OSTEP "processes-shell" project)
#
#   make            build ./wish
#   make test       run the official OSTEP test suite (fetched on first use)
#   make clean      remove build artifacts
#   make distclean  also remove the fetched test suite

CFLAGS = -std=gnu11 -Wall -Wextra -Werror -O2 -g

SRC_DIR   := src
BUILD_DIR := build
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all test clean distclean

all: wish

wish: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# -MMD -MP generate .d files so that editing a header rebuilds its users
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

test: wish
	./scripts/run-tests.sh

clean:
	rm -rf $(BUILD_DIR) wish

distclean: clean
	rm -rf .ostep-projects

-include $(DEPS)
