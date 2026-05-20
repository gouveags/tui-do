CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers -Wno-sign-compare
LDFLAGS ?=

BUILD_DIR := build
TARGET := $(BUILD_DIR)/tui-do
SOURCES := src/main.c src/terminal.c src/ui.c
TEST_TARGET := $(BUILD_DIR)/test-main-menu
TEST_SOURCES := test/main_menu_test.c src/terminal.c src/ui.c

.PHONY: all run test clean

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

clean:
	rm -rf $(BUILD_DIR)
