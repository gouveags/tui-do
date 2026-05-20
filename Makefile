CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers -Wno-sign-compare
LINT_CFLAGS ?= $(CFLAGS) -Werror
LDFLAGS ?=

BUILD_DIR := build
TARGET := $(BUILD_DIR)/tui-do
SOURCES := src/main.c src/app.c src/storage.c src/terminal.c src/ui.c src/markdown.c
TEST_TARGET := $(BUILD_DIR)/test-main-menu
TEST_SOURCES := test/main_menu_test.c src/terminal.c src/ui.c src/markdown.c
APP_TEST_TARGET := $(BUILD_DIR)/test-app
APP_TEST_SOURCES := test/app_test.c src/app.c src/storage.c
TERMINAL_TEST_TARGET := $(BUILD_DIR)/test-terminal
TERMINAL_TEST_SOURCES := test/terminal_test.c src/terminal.c
STORAGE_TEST_TARGET := $(BUILD_DIR)/test-storage
STORAGE_TEST_SOURCES := test/storage_test.c src/storage.c
MARKDOWN_TEST_TARGET := $(BUILD_DIR)/test-markdown
MARKDOWN_TEST_SOURCES := test/markdown_test.c src/markdown.c
FORMAT_FILES := .gitignore AGENTS.md README.md docs/vision-and-requirements.md Makefile src/app.c src/app.h src/main.c src/markdown.c src/markdown.h src/storage.c src/storage.h src/terminal.c src/terminal.h src/ui.c src/ui.h test/app_test.c test/main_menu_test.c test/markdown_test.c test/storage_test.c test/terminal_test.c test/test.h

.PHONY: all run test lint format format-check smoke check clean

all: $(TARGET)

$(TARGET): $(SOURCES) src/terminal.h src/ui.h vendor/clay/clay.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET) $(APP_TEST_TARGET) $(TERMINAL_TEST_TARGET) $(STORAGE_TEST_TARGET) $(MARKDOWN_TEST_TARGET)
	./$(TEST_TARGET)
	./$(APP_TEST_TARGET)
	./$(TERMINAL_TEST_TARGET)
	./$(STORAGE_TEST_TARGET)
	./$(MARKDOWN_TEST_TARGET)

$(TEST_TARGET): $(TEST_SOURCES) test/test.h src/terminal.h src/ui.h vendor/clay/clay.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SOURCES) -o $(TEST_TARGET) $(LDFLAGS)

$(APP_TEST_TARGET): $(APP_TEST_SOURCES) test/test.h src/app.h src/terminal.h src/ui.h vendor/clay/clay.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(APP_TEST_SOURCES) -o $(APP_TEST_TARGET) $(LDFLAGS)

$(TERMINAL_TEST_TARGET): $(TERMINAL_TEST_SOURCES) test/test.h src/terminal.h vendor/clay/clay.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TERMINAL_TEST_SOURCES) -o $(TERMINAL_TEST_TARGET) $(LDFLAGS)

$(STORAGE_TEST_TARGET): $(STORAGE_TEST_SOURCES) test/test.h src/storage.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(STORAGE_TEST_SOURCES) -o $(STORAGE_TEST_TARGET) $(LDFLAGS)

$(MARKDOWN_TEST_TARGET): $(MARKDOWN_TEST_SOURCES) test/test.h src/markdown.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(MARKDOWN_TEST_SOURCES) -o $(MARKDOWN_TEST_TARGET) $(LDFLAGS)

lint:
	$(CC) $(LINT_CFLAGS) -fsyntax-only $(SOURCES)
	$(CC) $(LINT_CFLAGS) -fsyntax-only $(TEST_SOURCES)
	$(CC) $(LINT_CFLAGS) -fsyntax-only $(APP_TEST_SOURCES)
	$(CC) $(LINT_CFLAGS) -fsyntax-only $(TERMINAL_TEST_SOURCES)
	$(CC) $(LINT_CFLAGS) -fsyntax-only $(STORAGE_TEST_SOURCES)
	$(CC) $(LINT_CFLAGS) -fsyntax-only $(MARKDOWN_TEST_SOURCES)

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
