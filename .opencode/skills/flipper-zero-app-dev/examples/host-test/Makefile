CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -O2 -g
SAN ?= -fsanitize=address,undefined

SRC := logic/timer_state.c
TEST_SRC := tests/test_main.c
BIN := build/tests

.PHONY: test clean
test:
	@mkdir -p build
	$(CC) $(CFLAGS) $(SAN) $(SRC) $(TEST_SRC) -o $(BIN)
	./$(BIN)

clean:
	rm -rf build
