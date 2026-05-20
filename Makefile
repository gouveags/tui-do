CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers -Wno-sign-compare
LINT_CFLAGS ?= $(CFLAGS) -Werror
LDFLAGS ?=

BUILD_DIR := build
TARGET := $(BUILD_DIR)/tui-do
SOURCES := src/main.c src/terminal.c src/ui.c
TEST_TARGET := $(BUILD_DIR)/test-main-menu
TEST_SOURCES := test/main_menu_test.c src/terminal.c src/ui.c
FORMAT_FILES := AGENTS.md README.md Makefile src/main.c src/terminal.c src/terminal.h src/ui.c src/ui.h test/main_menu_test.c test/test.h

.PHONY: all run test lint format format-check smoke check clean

all: $(TARGET)

$(TARGET): $(SOURCES) src/terminal.h src/ui.h vendor/clay/clay.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SOURCES) test/test.h src/terminal.h src/ui.h vendor/clay/clay.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SOURCES) -o $(TEST_TARGET) $(LDFLAGS)

lint:
	$(CC) $(LINT_CFLAGS) -fsyntax-only $(SOURCES)
	$(CC) $(LINT_CFLAGS) -fsyntax-only $(TEST_SOURCES)

format:
	perl -pi -e 's/[ \t]+$$//' $(FORMAT_FILES)

format-check:
	@if grep -n '[[:blank:]]$$' $(FORMAT_FILES); then \
		echo "Trailing whitespace found. Run make format."; \
		exit 1; \
	fi
	@for file in $(FORMAT_FILES); do \
		if [ -s "$$file" ] && [ "$$(tail -c 1 "$$file")" != "" ]; then \
			echo "$$file: missing final newline"; \
			exit 1; \
		fi; \
	done

smoke: $(TARGET)
	env COLUMNS=160 LINES=50 sh -c 'printf q | ./$(TARGET) >/dev/null'

check: format-check lint test all smoke

clean:
	rm -rf $(BUILD_DIR)
