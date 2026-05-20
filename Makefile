CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers -Wno-sign-compare
LDFLAGS ?=

BUILD_DIR := build
TARGET := $(BUILD_DIR)/tui-do
SOURCES := src/main.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES) vendor/clay/clay.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
