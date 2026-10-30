# Host-тесты чистого ядра FN Tester (этап R0).
#
# Собирается только код, не зависящий от SDK: lib/fn/core, lib/fn/proto и
# GUI-независимая часть ui/ (fn_format.c). Include-пути ui/views, ui/scenes,
# ui/fn_font.* и любые пути SDK запрещены (specs/001-fn-tester-ux/contracts/
# module-boundaries.md §2).
#
# Использование:
#   make -C tests/host              # собрать и прогнать все host-тесты
#   make -C tests/host asan         # то же под AddressSanitizer/UBSan
#   make -C tests/host check-layers # статическая проверка слоёв
#   make -C tests/host clean

CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -O2 -g

HOST_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
ROOT := $(abspath $(HOST_DIR)/../..)
BUILD_DIR := $(HOST_DIR)/build

# Только чистые слои: без путей SDK (contracts/module-boundaries.md §1).
INCLUDES := \
	-I$(HOST_DIR) \
	-I$(ROOT)/lib/fn/core \
	-I$(ROOT)/lib/fn/proto \
	-I$(ROOT)/ui

# Чистое ядро и host-тестируемая часть UI (появляются на этапах R1–R2).
LIB_SRC := \
	$(wildcard $(ROOT)/lib/fn/core/*.c) \
	$(wildcard $(ROOT)/lib/fn/proto/*.c) \
	$(wildcard $(ROOT)/ui/fn_format.c)

# Один бинарник на файл test_*.c — тесты не мешают друг другу.
TEST_SRC := $(sort $(wildcard $(HOST_DIR)/test_*.c))
TEST_BINS := $(patsubst $(HOST_DIR)/%.c,$(BUILD_DIR)/%,$(TEST_SRC))
TEST_BINS_ASAN := $(addsuffix -asan,$(TEST_BINS))

.DEFAULT_GOAL := test

.PHONY: test asan check-layers clean

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Любой провал компиляции или проверки — ненулевой код возврата.
$(BUILD_DIR)/%: $(HOST_DIR)/%.c $(LIB_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIB_SRC) -o $@

$(BUILD_DIR)/%-asan: $(HOST_DIR)/%.c $(LIB_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer \
		$(INCLUDES) $< $(LIB_SRC) -o $@

test: $(TEST_BINS)
	@failed=0; \
	for bin in $(TEST_BINS); do \
		echo "== $$(basename $$bin)"; \
		$$bin || failed=1; \
	done; \
	if [ $$failed -ne 0 ]; then echo "HOST TESTS: FAILED"; exit 1; fi; \
	echo "HOST TESTS: PASSED"

asan: $(TEST_BINS_ASAN)
	@failed=0; \
	for bin in $(TEST_BINS_ASAN); do \
		echo "== $$(basename $$bin)"; \
		$$bin || failed=1; \
	done; \
	if [ $$failed -ne 0 ]; then echo "HOST TESTS (ASAN): FAILED"; exit 1; fi; \
	echo "HOST TESTS (ASAN): PASSED"

check-layers:
	@$(HOST_DIR)/check_layers.sh

clean:
	rm -rf $(BUILD_DIR)
